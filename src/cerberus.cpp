#include "cerberus.h"

#include <openssl/conf.h>
#include <openssl/ssl.h>

#include "core/cerberuscore.h"
#include "core/cerberusregister.h"
#include "define.h"
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
msg_ptr Cerberus::stdTemplate(HASH32 id)
{
    auto tmplt = Message::create(id);

    switch (id)
    {
        case CERBERUS_MESSAGE_LOG_ID:
            tmplt->addSlotType<StringSlot>();
            break;

        case CERBERUS_MESSAGE_TERM_ID:
            // nothing to add
            break;

        case CERBERUS_MESSAGE_TASK_ID:
            tmplt->addSlotType<UInt64Slot>("client");
            tmplt->addSlotType<TaskSlot>("task");
            break;

        case CERBERUS_MESSAGE_TASKEND_ID:
            tmplt->addSlotType<ResultSlot>("result");
            tmplt->addSlotType<VoidPSlot>("player");
            break;

        case CERBERUS_MESSAGE_SOCKETDATA_ID:
            tmplt->addSlotType<ResultSlot>("result");
            tmplt->addSlotType<HostSlot>("host");
            tmplt->addSlotType<BufferSlot>("buffer");
            break;

            // add here more message specializations..

        default:
            throw cImplMissExc("Requested standard message is not defined");
    }

    return tmplt;
}
//=============================================================================
void Cerberus::registerObj(Recordable* object)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    Cerberus::framework.core.data->registerObj(object);
}
//=============================================================================
void Cerberus::unregisterObj(HASH32 id)
{
    if (!Cerberus::framework.core.isReady()) return;
    auto locker = Cerberus::framework.core.getLocker();
    Cerberus::framework.core.data->unregisterObj(id);
}
//=============================================================================
OpRes Cerberus::sendMsgToObj(HASH32 id, msg_ptr& msg)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->sendMsgToObj(id, msg);
}
//=============================================================================
OpRes Cerberus::sendMsgToObj_deep(HASH32 id, const msg_ptr& msg)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->sendMsgToObj_deep(id, msg);
}
//=============================================================================
HASH32 Cerberus::addPlugin(void* handle, const std::string& path, bool& exists)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->addPlugin(handle, path, exists);
}
//=============================================================================
MutexLocker Cerberus::getPluginMutex(HASH32 id)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->getPluginMutex(id);
}
//=============================================================================
void* Cerberus::checkPlugin(HASH32 id)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->checkPlugin(id);
}
//=============================================================================
bool Cerberus::updatePlugin(uint32_t id, const std::string& path, void* handle)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->updatePlugin(id, path, handle);
}
//=============================================================================
void Cerberus::stopTimer(std::atomic_bool& bit)
{
    if (!Cerberus::framework.core.isReady())
    {
        bit.store(false, std::memory_order_relaxed);
        return;
    }

    auto locker = Cerberus::framework.core.getLocker();
    Cerberus::framework.core.data->m_eventScheduler.stopTimer(bit);
}
//=============================================================================
void Cerberus::startTimer(TimerData& data)
{
    if (!Cerberus::framework.core.isReady()) return;

    auto locker = Cerberus::framework.core.getLocker();
    Cerberus::framework.core.data->m_eventScheduler.startTimer(data);
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
OpRes Cerberus::send_deep(const msg_ptr& message, HASH32 recipientID)
{
    if (!message) return OR_WrongArgument;

    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();

    // deep-copy is done only if accepted (inside core/recipient)
    return Cerberus::framework.core.data->send_deep(message, recipientID);
}
//=============================================================================
OpRes Cerberus::send_deep(const msg_ptr& message, const std::string& recipient)
{
    if (!message) return OR_WrongArgument;

    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();

    return Cerberus::framework.core.data->send_deep(message, idByName(recipient));
}
//=============================================================================
OpRes Cerberus::send(msg_ptr& message, HASH32 recipientID)
{
    if (!message) return OR_WrongArgument;

    if (recipientID != CERBERUS_INVALID_ID) message->setRecipient(recipientID);

    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();

    return Cerberus::framework.core.data->send(message);
}
//=============================================================================
OpRes Cerberus::send(msg_ptr& message, const std::string& recipient)
{
    if (!message) return OR_WrongArgument;

    message->setRecipient(idByName(recipient));

    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();

    return Cerberus::framework.core.data->send(message);
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
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->objIdByName(name);
}
//=============================================================================
OpRes Cerberus::registerTemplate(const msg_ptr& tmplt)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();

    return Cerberus::framework.core.data->addMsgTemplate(tmplt);
}
//=============================================================================
msg_ptr Cerberus::constructMessage(HASH32 id)
{
    if (id == CERBERUS_INVALID_ID) return Message::create();

    if (id < CERBERUS_REG_RANGE_START) return stdTemplate(id);

    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();

    auto msg = Cerberus::framework.core.data->constructMessage(id);

    if (!msg) return Message::create();

    return std::move(msg);
}
//=============================================================================
msg_ptr Cerberus::constructMessage(const std::string& name) { return constructMessage(hashFunc_res(name)); }
//=============================================================================
//=============================FrameworkData===================================
//=============================================================================
void FrameworkData::construct(const CerberusInitConf& conf)
{
    if (core.isReady()) throw cUsageErrorExc("attempt to construct the framework multiple times");

    log.construct();
    log.data->setup(conf.logSetup);
    core.construct();
    core.data->setup(conf.coreSetup);
}
//=============================================================================
void FrameworkData::destroy()
{
    if (!core.isReady()) throw cUsageErrorExc("attempt to destroy the framework multiple times");
    core.destroy();
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
    if (core.isReady())
    {
        auto locker = core.getLocker();
        core.data->cleanupPlugins();
    }
    core.data->join(true).expect("Unable to join the core Thread");
    log.data->stop();
}
//=============================================================================
