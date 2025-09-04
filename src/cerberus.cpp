#include "cerberus.h"

#include <openssl/conf.h>
#include <openssl/ssl.h>

#include "core/cerberuscore.h"
#include "core/cerberusregister.h"
#include "exception/exception.h"

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
void Cerberus::registerObj(Recordable* object)
{
    Cerberus::framework.reg.isReadySevere();
    auto locker = Cerberus::framework.reg.getLocker();
    Cerberus::framework.reg.data->registerObj(object);
}
//=============================================================================
void Cerberus::unregisterObj(HASH32 id)
{
    if (!Cerberus::framework.reg.isReady()) return;
    auto locker = Cerberus::framework.reg.getLocker();
    Cerberus::framework.reg.data->unregisterObj(id);
}
//=============================================================================
void Cerberus::sendMsgToObj(HASH32 id, msg_ptr msg)
{
    Cerberus::framework.reg.isReadySevere();
    auto locker = Cerberus::framework.reg.getLocker();
    Cerberus::framework.reg.data->sendMsgToObj(id, std::move(msg));
}
//=============================================================================
void Cerberus::sendMsgToObj(const std::string& name, msg_ptr msg)
{
    Cerberus::framework.reg.isReadySevere();
    auto locker = Cerberus::framework.reg.getLocker();
    Cerberus::framework.reg.data->sendMsgToObj(name, std::move(msg));
}
//=============================================================================
HASH32 Cerberus::addPlugin(void* handle, const std::string& path, bool& exists)
{
    Cerberus::framework.reg.isReadySevere();
    auto locker = Cerberus::framework.reg.getLocker();
    return Cerberus::framework.reg.data->addPlugin(handle, path, exists);
}
//=============================================================================
MutexLocker Cerberus::getPluginMutex(HASH32 id)
{
    Cerberus::framework.reg.isReadySevere();
    auto locker = Cerberus::framework.reg.getLocker();
    return Cerberus::framework.reg.data->getPluginMutex(id);
}
//=============================================================================
void* Cerberus::checkPlugin(HASH32 id)
{
    Cerberus::framework.reg.isReadySevere();
    auto locker = Cerberus::framework.reg.getLocker();
    return Cerberus::framework.reg.data->checkPlugin(id);
}
//=============================================================================
bool Cerberus::updatePlugin(uint32_t id, const std::string& path, void* handle)
{
    Cerberus::framework.reg.isReadySevere();
    auto locker = Cerberus::framework.reg.getLocker();
    return Cerberus::framework.reg.data->updatePlugin(id, path, handle);
}
//=============================================================================
void Cerberus::startTimer(TimerData& data)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    Cerberus::framework.core.data->m_eventScheduler.startTimer(data);
}
//=============================================================================
void Cerberus::stopTimer(std::atomic_bool& bit)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    Cerberus::framework.core.data->m_eventScheduler.stopTimer(bit);
}
//=============================================================================
//=================================INTERFACE===================================
//=============================================================================
void Cerberus::init(const CerberusInitConf& parms)
{
    Cerberus::framework.construct(parms);

    Cerberus::framework.start();

    logDebug("Cerberus init completed");

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
CerberusInitConf Cerberus::cerberusDefaultParms()
{
    CerberusInitConf toReturn{};
    toReturn.useCiphers               = true;
    toReturn.logSetup.colorFormatting = true;

    toReturn.logSetup.fileLogConf.enable        = true;
    toReturn.logSetup.fileLogConf.fileName      = "./app.log";
    toReturn.logSetup.fileLogConf.fileMaxSize   = 4096;
    toReturn.logSetup.fileLogConf.logDir        = "logs";
    toReturn.logSetup.fileLogConf.fileNameFmt   = "%Y-%M-%D_%h-%m-%s.log";
    toReturn.logSetup.fileLogConf.logDirMaxSize = 1000000;

    toReturn.logSetup.appLogLevel  = LL_Error;
    toReturn.logSetup.cerbLogLevel = LL_Error;

    toReturn.coreSetup.threadPool          = 0;
    toReturn.coreSetup.backupThreadMaxTime = 10000;  // 10 seconds

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
            throw cImplMissExc("Version type mismatch");
    }

    ret.text = CerberusUtils::strPrint("%u.%u.%u%s", ret.major, ret.minor, ret.patch, typ.c_str());

    return ret;
}
//=============================================================================
void Cerberus::log(const std::string& str, LogLevel logLevel, const std::string& author, bool application)
{
    if (!Cerberus::framework.log.isReady()) return;  // abort log
    auto locker = Cerberus::framework.log.getLocker();
    Cerberus::framework.log.data->log(str, logLevel, author, application);
}
//=============================================================================
void Cerberus::send(msg_ptr& message, HASH32 recipientID)
{
    if (recipientID != CERBERUS_INVALID_ID) message->setRecipient(recipientID);
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    Cerberus::framework.core.data->send(std::move(message));
}
//=============================================================================
void Cerberus::send(msg_ptr& message, const std::string& recipient)
{
    message->setRecipient(idByName(recipient));
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    Cerberus::framework.core.data->send(std::move(message));
}
//=============================================================================
OpResData<CHANDLE> Cerberus::newSocket(const SocketSettings& settings)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->newSocket(settings);
}
//=============================================================================
OpRes Cerberus::addSocketListener(CHANDLE socket, HASH32 threadID)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->addSocketListener(socket, threadID);
}
//=============================================================================
OpRes Cerberus::addSocketListener(CHANDLE socket, const std::string& threadName)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->addSocketListener(socket, idByName(threadName));
}
//=============================================================================
OpRes Cerberus::socketSend(CHANDLE socket, const ByteBuffer& buffer)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->socketSend(socket, buffer);
}
//=============================================================================
OpRes Cerberus::removeSocket(CHANDLE socket)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->removeSocket(socket);
}
//=============================================================================
HASH32 Cerberus::idByName(const std::string& name)
{
    Cerberus::framework.reg.isReadySevere();
    auto locker = Cerberus::framework.reg.getLocker();
    return Cerberus::framework.reg.data->objIdByName(name);
}
//=============================================================================
OpResData<MessageTemplate> Cerberus::templateById(uint32_t id)
{
    if (id < CERBERUS_REG_RANGE_START) return CerberusUtils::standardTemplate(id);

    Cerberus::framework.reg.isReadySevere();
    auto locker = Cerberus::framework.reg.getLocker();
    return Cerberus::framework.reg.data->msgTemplateById(id);
}
//=============================================================================
OpResData<MessageTemplate> Cerberus::templateByName(const std::string& name)
{
    Cerberus::framework.reg.isReadySevere();
    auto locker = Cerberus::framework.reg.getLocker();
    return Cerberus::framework.reg.data->msgTemplateByName(name);
}
//=============================================================================
HASH32 Cerberus::msgIdByName(const std::string& name)
{
    Cerberus::framework.reg.isReadySevere();
    auto locker = Cerberus::framework.reg.getLocker();
    return Cerberus::framework.reg.data->msgTemplateByName(name).expect().value.id;
}
//=============================================================================
OpResData<HASH32> Cerberus::registerMessage(const Message& message, const std::string& name)
{
    auto tmplt = MessageTemplate(message, name);

    Cerberus::framework.reg.isReadySevere();
    auto locker = Cerberus::framework.reg.getLocker();

    return Cerberus::framework.reg.data->addMsgTemplate(tmplt);
}
//=============================================================================
OpResData<HASH32> Cerberus::registerTemplate(const MessageTemplate& tmplt)
{
    Cerberus::framework.reg.isReadySevere();
    auto locker = Cerberus::framework.reg.getLocker();

    return Cerberus::framework.reg.data->addMsgTemplate(tmplt);
}
//=============================================================================
msg_ptr Cerberus::constructMessage(HASH32 id)
{
    if (id == CERBERUS_INVALID_ID) return Message::create();

    auto tmplt = Cerberus::templateById(id);

    if (tmplt.fail()) return Message::create();

    return tmplt.value.instantiate();
}
//=============================================================================
msg_ptr Cerberus::constructMessage(const std::string& name)
{
    if (name.empty()) return Message::create();

    auto tmplt = Cerberus::templateByName(name);

    if (tmplt.fail()) return Message::create();

    return tmplt.value.instantiate();
}
//=============================================================================
//=============================FrameworkData===================================
//=============================================================================
void FrameworkData::construct(const CerberusInitConf& conf)
{
    if (core.isReady()) throw cUsageErrorExc("attempt to construct the framework multiple times");

    log.construct();
    log.data->setup(conf.logSetup);

    reg.construct();
    core.construct();
    core.data->setup(conf.coreSetup);
}
//=============================================================================
void FrameworkData::destroy()
{
    if (!core.isReady()) throw cUsageErrorExc("attempt to destroy the framework multiple times");
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
