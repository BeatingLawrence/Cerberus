#include "cerberus.h"

#include <openssl/conf.h>
#include <openssl/ssl.h>

#include "core/cerberuscore.h"
#include "core/cerberusregister.h"
#include "exception/exception.h"
#include "message/slot/slots.h"

#ifdef WINDOWS_SYSTEM
#include <windows.h>
#else
#include <signal.h>
#include <unistd.h>
#endif

#define CANNOT_USE "cannot use the framework before the init() call"

#if ((CERBERUS_VERSION_TWEAK != 0) && (CERBERUS_VERSION_TWEAK != 1) && (CERBERUS_VERSION_TWEAK != 99))
#error "WRONG VERSION TYPE"
#endif

using namespace cerberus;
using namespace cerberus::core;

SingularCerberusData Cerberus::singularCerbData;

//=============================================================================
void Cerberus::registerObj(CerberusObject *object) { return Cerberus::singularCerbData.getReg()->registerObj(object); }
//=============================================================================
void Cerberus::unregisterObj(uint32_t id) { return Cerberus::singularCerbData.getReg()->unregisterObj(id); }
//=============================================================================
void Cerberus::sendMsgToObj(uint32_t id, cerberus_message msg) { return Cerberus::singularCerbData.getReg()->sendMsgToObj(id, msg); }
//=============================================================================
message::MessageTemplate Cerberus::msgTemplateById(uint32_t id) { return Cerberus::singularCerbData.getReg()->msgTemplateById(id); }
//=============================================================================
message::MessageTemplate Cerberus::msgTemplateByName(const std::string &name) { return Cerberus::singularCerbData.getReg()->msgTemplateByName(name); }
//=============================================================================
uint32_t Cerberus::registerMessage(const message::Message &message, const std::string &name)
{
    auto tmplt = cerberus::message::MessageTemplate(message, name);
    tmplt.checkIn();
    return tmplt.id();
}
//=============================================================================
cerberus_message Cerberus::messageConstruct(uint32_t id)
{
    if (id == CERBERUS_INVALID_ID)
    {
        return message::Message::create();
    }

    if (id < CERBERUS_FACTORY_START_ID)
    {
        // reserved range
        logError("The requested ID is in the reserved range");
        return message::Message::create();
    }

    auto tmplt = cerberus::Cerberus::msgTemplateById(id);

    if (!tmplt.isObjValid())
    {
        return message::Message::create();
    }

    cerberus_message message = message::Message::create(id);

    for (size_t i = 0; i < tmplt.count(); i++)
    {
        message->addSlot(CerberusUtils::newSlot(tmplt.getSlotTypeAt(i)));
    }

    return message;
}
//=============================================================================
cerberus_message Cerberus::messageConstruct(const std::string &name)
{
    auto tmplt = Cerberus::msgTemplateByName(name);

    if (!tmplt.isObjValid())
    {
        return message::Message::create();
    }

    cerberus_message message = message::Message::create(tmplt.id());

    for (size_t i = 0; i < tmplt.count(); i++)
    {
        message->addSlot(CerberusUtils::newSlot(tmplt.getSlotTypeAt(i)));
    }

    return message;
}
//=============================================================================
cerberus_message Cerberus::standardMessageConstruct(StandardMessage type)
{
    cerberus_message msg;

    switch (type)
    {
        case SM_LogMsg:
            msg = message::Message::create(CERBERUS_MESSAGE_LOG_ID);
            msg->addSlot(message::slot::StringSlot::create());
            break;

        case SM_TerminationMsg:
            msg = message::Message::create(CERBERUS_MESSAGE_TERM_ID);
            break;

            // add here more message specializations..

        default:
            logError("Given type does not exist");
            msg = message::Message::create(CERBERUS_INVALID_ID);
            break;
    }

    return msg;
}
//=============================================================================
uint32_t Cerberus::addPlugin(void *handle, const std::string &path, bool &exists) { return Cerberus::singularCerbData.getReg()->addPlugin(handle, path, exists); }
//=============================================================================
mutex::MutexLocker Cerberus::getPluginMutex(uint32_t id) { return Cerberus::singularCerbData.getReg()->getPluginMutex(id); }
//=============================================================================
void *Cerberus::checkPlugin(uint32_t id) { return Cerberus::singularCerbData.getReg()->checkPlugin(id); }
//=============================================================================
bool Cerberus::updatePlugin(uint32_t id, const std::string &path, void *handle) { return Cerberus::singularCerbData.getReg()->updatePlugin(id, path, handle); }
//=============================================================================
void Cerberus::startTimer(std::atomic_bool &bit, time::TimeFrame t, timerCallback callback) { return Cerberus::singularCerbData.getCore()->m_eventScheduler.startTimer(bit, t, callback); }
//=============================================================================
void Cerberus::startTimer(std::atomic_bool &bit, time::DateTime d, time::TimeFrame t, timerCallback callback) { return Cerberus::singularCerbData.getCore()->m_eventScheduler.startTimer(bit, d, t, callback); }
//=============================================================================
void Cerberus::startTimer(std::atomic_bool &bit, time::DateTime d, timerCallback callback) { return Cerberus::singularCerbData.getCore()->m_eventScheduler.startTimer(bit, d, callback); }
//=============================================================================
void Cerberus::stopTimer(std::atomic_bool &bit) { return Cerberus::singularCerbData.getCore()->m_eventScheduler.stopTimer(bit); }
//=============================================================================
void Cerberus::init(const CerberusInitParms &parms)
{
    singularCerbData.constructLog();
    singularCerbData.getLog()->setup(parms.logSetup);
    singularCerbData.constructReg();
    singularCerbData.constructCore();

    singularCerbData.getLog()->start();
    singularCerbData.getCore()->start();

    logInfo("Cerberus init completed");

    if (parms.useCiphers)
    {
        SSL_library_init();
        SSL_load_error_strings();
    }

#ifndef WINDOWS_SYSTEM
    // PostgreSQL SIGPIPE Ignoring:
    struct sigaction action = {};
    action.sa_handler       = SIG_IGN;

    if (sigaction(SIGPIPE, &action, nullptr) != 0)
    {
        logError("Unable to ignore SIGPIPE system signal, using SQL may terminate the process");
    }

#endif
}
//=============================================================================
void Cerberus::deinit()
{
    singularCerbData.destroyCore();
    singularCerbData.getLog()->stop();
    singularCerbData.destroyReg();
    singularCerbData.destroyLog();
}
//=============================================================================
CerberusInitParms Cerberus::cerberusDefaultParms()
{
    CerberusInitParms toReturn{};
    toReturn.logSetup.colorFormatting     = true;
    toReturn.logSetup.logOnFile           = true;
    toReturn.logSetup.logFileName         = "./last.log";
    toReturn.logSetup.logFileMaximumSize  = 4096;  // 4Kb
    toReturn.logSetup.applicationLogLevel = LL_Error;
    toReturn.logSetup.cerberusLogLevel    = LL_Error;

#ifdef WINDOWS_SYSTEM
    toReturn.logSetup.infoRole.foregroundColor    = TERMINAL_FOREGROUND_GREEN;
    toReturn.logSetup.warningRole.foregroundColor = (TERMINAL_FOREGROUND_GREEN | TERMINAL_FOREGROUND_RED);
    toReturn.logSetup.errorRole.foregroundColor   = TERMINAL_FOREGROUND_RED;
    toReturn.logSetup.debugRole.foregroundColor   = (TERMINAL_FOREGROUND_RED | TERMINAL_FOREGROUND_BLUE);
#else
    toReturn.logSetup.infoRole.backgroundColor    = TERMINAL_BACKGROUND_BLACK;
    toReturn.logSetup.warningRole.backgroundColor = TERMINAL_BACKGROUND_BLACK;
    toReturn.logSetup.errorRole.backgroundColor   = TERMINAL_BACKGROUND_BLACK;
    toReturn.logSetup.debugRole.backgroundColor   = TERMINAL_BACKGROUND_BLACK;
    toReturn.logSetup.infoRole.foregroundColor    = TERMINAL_FOREGROUND_GREEN;
    toReturn.logSetup.warningRole.foregroundColor = TERMINAL_FOREGROUND_YELLOW;
    toReturn.logSetup.errorRole.foregroundColor   = TERMINAL_FOREGROUND_RED;
    toReturn.logSetup.debugRole.foregroundColor   = TERMINAL_FOREGROUND_MAGENTA;
#endif
    return toReturn;
}
//=============================================================================
CerbVersion Cerberus::cerberusVersion()
{
    CerbVersion ret;
    std::string ver = CERBERUS_VERSION;
    std::string typ;

    auto splitted = CerberusUtils::split(ver, ".");
    ret.major     = CerberusUtils::stringToInt(splitted.left);
    splitted      = CerberusUtils::split(splitted.right, ".");
    ret.minor     = CerberusUtils::stringToInt(splitted.left);
    splitted      = CerberusUtils::split(splitted.right, ".");
    ret.patch     = CerberusUtils::stringToInt(splitted.left);

    switch (CerberusUtils::stringToInt(splitted.right))
    {
        case 0:
            ret.type = CerbVersion::Alpha;
            typ      = "a";
            break;

        case 1:
            ret.type = CerbVersion::Beta;
            typ      = "b";
            break;

        case 99:
            ret.type = CerbVersion::Release;
            break;

        default:
            throw cerberusImplMissExc("Version type mismatch");
    }

    ret.text = CerberusUtils::strPrint("%u.%u.%u%s", ret.major, ret.minor, ret.patch, typ.c_str());

    return ret;
}
//=============================================================================
void Cerberus::log(const std::string &str, LogLevel logLevel, const std::string &author, bool application) { Cerberus::singularCerbData.getLog()->log(str, logLevel, author, application); }
//=============================================================================
uint32_t Cerberus::objIdByName(const std::string &name) { return Cerberus::singularCerbData.getReg()->objIdByName(name); }
//=============================================================================
void Cerberus::send(cerberus_message message) { Cerberus::singularCerbData.getCore()->addMessage(message); }
//=============================================================================
void Cerberus::send(cerberus_message message, uint32_t id)
{
    message->setDestination(id);
    Cerberus::singularCerbData.getCore()->addMessage(message);
}
//=============================================================================
void Cerberus::send(cerberus_message message, const std::string &name)
{
    message->setDestination(objIdByName(name));
    Cerberus::singularCerbData.getCore()->addMessage(message);
}
//=============================================================================
void SingularCerberusData::constructLog()
{
    // wait for build operations to be finished
    while (logWorking.test_and_set())
    {
    }

    if (core) throw cerberusUsageErrorExc(CANNOT_USE);

    log = new CerberusLog;

    logWorking.clear();
}
//=============================================================================
void SingularCerberusData::constructReg()
{
    // wait for build operations to be finished
    while (regWorking.test_and_set())
    {
    }

    if (reg) throw cerberusUsageErrorExc(CANNOT_USE);

    reg = new CerberusRegister;

    regWorking.clear();
}
//=============================================================================
void SingularCerberusData::constructCore()
{
    // wait for build operations to be finished
    while (coreWorking.test_and_set())
    {
    }

    if (core) throw cerberusUsageErrorExc(CANNOT_USE);

    core = new CerberusCore;

    coreWorking.clear();
}
//=============================================================================
void SingularCerberusData::destroyLog()
{
    // wait for build or use operations to be finished
    while (logWorking.test_and_set())
    {
    }

    if (!log) return;

    delete log;
    log = nullptr;
    logWorking.clear();
}
//=============================================================================
void SingularCerberusData::destroyReg()
{
    // wait for build or use operations to be finished
    while (regWorking.test_and_set())
    {
    }

    if (!reg) return;

    reg->cleanupPlugins();

    delete reg;
    reg = nullptr;
    regWorking.clear();
}
//=============================================================================
void SingularCerberusData::destroyCore()
{
    // wait for build or use operations to be finished
    while (coreWorking.test_and_set())
    {
    }

    if (!core) return;

    core->join(true).expect("Unable to join the core Thread");

    delete core;
    core = nullptr;
    coreWorking.clear();
}
//=============================================================================
CerberusCore *SingularCerberusData::getCore()
{
    // wait for build operations to be finished
    while (coreWorking.test())
    {
    }

    if (!core) throw cerberusUsageErrorExc(CANNOT_USE);

    return core;
}
//=============================================================================
CerberusRegister *SingularCerberusData::getReg()
{
    // wait for build operations to be finished
    while (regWorking.test())
    {
    }

    if (!reg) throw cerberusUsageErrorExc(CANNOT_USE);

    return reg;
}
//=============================================================================
CerberusLog *SingularCerberusData::getLog()
{
    // wait for build operations to be finished
    while (logWorking.test())
    {
    }

    if (!log) throw cerberusUsageErrorExc(CANNOT_USE);

    return log;
}
//=============================================================================
