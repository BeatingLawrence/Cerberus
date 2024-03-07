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

#if ((CERBERUS_VERSION_TWEAK != 0) && (CERBERUS_VERSION_TWEAK != 1) && (CERBERUS_VERSION_TWEAK != 99))
#error "WRONG VERSION TYPE"
#endif

using namespace cerberus;
using namespace cerberus::core;

FrameworkData Cerberus::framework;

//=============================================================================
void Cerberus::registerObj(CerberusObject *object)
{
    Cerberus::framework.reg.isReadySevere();
    Cerberus::framework.reg.begin();
    Cerberus::framework.reg.data->registerObj(object);
    Cerberus::framework.reg.end();
}
//=============================================================================
void Cerberus::unregisterObj(uint32_t id)
{
    if (!Cerberus::framework.reg.isReady()) return;
    Cerberus::framework.reg.begin();
    Cerberus::framework.reg.data->unregisterObj(id);
    Cerberus::framework.reg.end();
}
//=============================================================================
void Cerberus::sendMsgToObj(uint32_t id, cerberus_message msg)
{
    Cerberus::framework.reg.isReadySevere();
    Cerberus::framework.reg.begin();
    Cerberus::framework.reg.data->sendMsgToObj(id, msg);
    Cerberus::framework.reg.end();
}
//=============================================================================
message::MessageTemplate Cerberus::msgTemplateById(uint32_t id)
{
    Cerberus::framework.reg.isReadySevere();
    Cerberus::framework.reg.begin();
    auto ret = Cerberus::framework.reg.data->msgTemplateById(id);
    Cerberus::framework.reg.end();
    return ret;
}
//=============================================================================
message::MessageTemplate Cerberus::msgTemplateByName(const std::string &name)
{
    Cerberus::framework.reg.isReadySevere();
    Cerberus::framework.reg.begin();
    auto ret = Cerberus::framework.reg.data->msgTemplateByName(name);
    Cerberus::framework.reg.end();
    return ret;
}
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
    if (id == CERBERUS_INVALID_ID || id < CERBERUS_FACTORY_START_ID)
    {
        throw cerberusIllegalArgExc("invalid id");
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
uint32_t Cerberus::addPlugin(void *handle, const std::string &path, bool &exists)
{
    Cerberus::framework.reg.isReadySevere();
    Cerberus::framework.reg.begin();
    auto ret = Cerberus::framework.reg.data->addPlugin(handle, path, exists);
    Cerberus::framework.reg.end();
    return ret;
}
//=============================================================================
mutex::MutexLocker Cerberus::getPluginMutex(uint32_t id)
{
    Cerberus::framework.reg.isReadySevere();
    Cerberus::framework.reg.begin();
    auto ret = Cerberus::framework.reg.data->getPluginMutex(id);
    Cerberus::framework.reg.end();
    return ret;
}
//=============================================================================
void *Cerberus::checkPlugin(uint32_t id)
{
    Cerberus::framework.reg.isReadySevere();
    Cerberus::framework.reg.begin();
    auto ret = Cerberus::framework.reg.data->checkPlugin(id);
    Cerberus::framework.reg.end();
    return ret;
}
//=============================================================================
bool Cerberus::updatePlugin(uint32_t id, const std::string &path, void *handle)
{
    Cerberus::framework.reg.isReadySevere();
    Cerberus::framework.reg.begin();
    auto ret = Cerberus::framework.reg.data->updatePlugin(id, path, handle);
    Cerberus::framework.reg.end();
    return ret;
}
//=============================================================================
void Cerberus::startTimer(std::atomic_bool &bit, time::TimeFrame t, timerCallback callback)
{
    Cerberus::framework.core.isReadySevere();
    Cerberus::framework.core.begin();
    Cerberus::framework.core.data->m_eventScheduler.startTimer(bit, t, callback);
    Cerberus::framework.core.end();
}
//=============================================================================
void Cerberus::startTimer(std::atomic_bool &bit, time::DateTime d, time::TimeFrame t, timerCallback callback)
{
    Cerberus::framework.core.isReadySevere();
    Cerberus::framework.core.begin();
    Cerberus::framework.core.data->m_eventScheduler.startTimer(bit, d, t, callback);
    Cerberus::framework.core.end();
}
//=============================================================================
void Cerberus::startTimer(std::atomic_bool &bit, time::DateTime d, timerCallback callback)
{
    Cerberus::framework.core.isReadySevere();
    Cerberus::framework.core.begin();
    Cerberus::framework.core.data->m_eventScheduler.startTimer(bit, d, callback);
    Cerberus::framework.core.end();
}
//=============================================================================
void Cerberus::stopTimer(std::atomic_bool &bit)
{
    Cerberus::framework.core.isReadySevere();
    Cerberus::framework.core.begin();
    Cerberus::framework.core.data->m_eventScheduler.stopTimer(bit);
    Cerberus::framework.core.end();
}
//=============================================================================
void Cerberus::init(const CerberusInitParms &parms)
{
    Cerberus::framework.construct(parms.logSetup);

    Cerberus::framework.start();

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
    Cerberus::framework.stop();
    Cerberus::framework.destroy();
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
    ret.major     = CerberusUtils::stringToInt(splitted.left).value;
    splitted      = CerberusUtils::split(splitted.right, ".");
    ret.minor     = CerberusUtils::stringToInt(splitted.left).value;
    splitted      = CerberusUtils::split(splitted.right, ".");
    ret.patch     = CerberusUtils::stringToInt(splitted.left).value;

    switch (CerberusUtils::stringToInt(splitted.right).value)
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
void Cerberus::log(const std::string &str, LogLevel logLevel, const std::string &author, bool application)
{
    if (!Cerberus::framework.log.isReady()) return;  // abort log
    Cerberus::framework.log.begin();
    Cerberus::framework.log.data->log(str, logLevel, author, application);
    Cerberus::framework.log.end();
}
//=============================================================================
uint32_t Cerberus::objIdByName(const std::string &name)
{
    Cerberus::framework.reg.isReadySevere();
    Cerberus::framework.reg.begin();
    auto ret = Cerberus::framework.reg.data->objIdByName(name);
    Cerberus::framework.reg.end();
    return ret;
}
//=============================================================================
void Cerberus::send(cerberus_message message)
{
    Cerberus::framework.core.isReadySevere();
    Cerberus::framework.core.begin();
    Cerberus::framework.core.data->addMessage(message);
    Cerberus::framework.core.end();
}
//=============================================================================
void Cerberus::send(cerberus_message message, uint32_t id)
{
    message->setDestination(id);
    Cerberus::framework.core.isReadySevere();
    Cerberus::framework.core.begin();
    Cerberus::framework.core.data->addMessage(message);
    Cerberus::framework.core.end();
}
//=============================================================================
void Cerberus::send(cerberus_message message, const std::string &name)
{
    message->setDestination(objIdByName(name));
    Cerberus::framework.core.isReadySevere();
    Cerberus::framework.core.begin();
    Cerberus::framework.core.data->addMessage(message);
    Cerberus::framework.core.end();
}
//=============================================================================
void FrameworkData::construct(const CerberusLogSetup &logSetup)
{
    if (core.isReady()) throw cerberusUsageErrorExc("attempt to construct the framework multiple times");

    log.construct();
    log.data->setup(logSetup);

    reg.construct();
    core.construct();
}
//=============================================================================
void FrameworkData::destroy()
{
    if (!core.isReady()) throw cerberusUsageErrorExc("attempt to destroy the framework multiple times");

    core.destroy();
    reg.destroy();
    log.destroy();
}
//=============================================================================
void FrameworkData::start()
{
    log.data->start();
    core.data->start();
}
//=============================================================================
void FrameworkData::stop()
{
    reg.data->cleanupPlugins();
    core.data->join(true).expect("Unable to join the core Thread");
    log.data->stop();
}
//=============================================================================
