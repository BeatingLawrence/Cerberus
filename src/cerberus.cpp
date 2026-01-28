#include "cerberus.h"

#include <openssl/conf.h>
#include <openssl/ssl.h>

#include <sstream>

#include "core/cerberuscore.h"
#include "core/cerberusregister.h"
#include "define.h"
#include "exception/exception.h"
#include "data/filesystem/inidatafile.h"

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

namespace
{
    const char* kFrameworkSection = "framework";

    bool readBoolOpt(IniDataFile& ini, const std::string& key, bool& out)
    {
        auto r = ini.read_bool(key, kFrameworkSection);
        if (r == OR_OK)
        {
            out = r.value;
            return true;
        }
        return false;
    }

    bool readIntOpt(IniDataFile& ini, const std::string& key, int64_t& out)
    {
        auto r = ini.read_integer(key, kFrameworkSection);
        if (r == OR_OK)
        {
            out = r.value;
            return true;
        }
        return false;
    }

    bool readDoubleOpt(IniDataFile& ini, const std::string& key, double& out)
    {
        auto r = ini.read_double(key, kFrameworkSection);
        if (r == OR_OK)
        {
            out = static_cast<double>(r.value);
            return true;
        }
        return false;
    }

    bool readStringOpt(IniDataFile& ini, const std::string& key, std::string& out)
    {
        auto r = ini.read_string(key, kFrameworkSection);
        if (r == OR_OK)
        {
            out = r.value;
            return true;
        }
        return false;
    }

    bool parseCoreSet(const std::string& text, CoreSet& out)
    {
        std::string s = text;
        for (char& c : s)
        {
            if (c == ',' || c == ';') c = ' ';
        }

        std::istringstream iss(s);
        int core = -1;
        bool any = false;
        while (iss >> core)
        {
            out.addCore(core);
            any = true;
        }
        return any;
    }

    void applyIniToInitConf(IniDataFile& ini, CerberusInitConf& conf)
    {
        int64_t ival = 0;
        double dval = 0.0;
        bool bval = false;
        std::string sval;

        if (readBoolOpt(ini, "useCiphers", bval)) conf.useCiphers = bval;
        if (readStringOpt(ini, "appConfigurationFile", sval)) conf.appConfigurationFile = sval;
        if (readBoolOpt(ini, "initFromFile", bval)) conf.initFromFile = bval;

        if (readIntOpt(ini, "logSetup.appLogLevel", ival))
            conf.logSetup.appLogLevel = static_cast<LogLevel>(ival);
        if (readIntOpt(ini, "logSetup.fwLogLevel", ival))
            conf.logSetup.fwLogLevel = static_cast<LogLevel>(ival);
        if (readBoolOpt(ini, "logSetup.colorFormatting", bval))
            conf.logSetup.colorFormatting = bval;

        if (readBoolOpt(ini, "logSetup.fileLogConf.enable", bval))
            conf.logSetup.fileLogConf.enable = bval;
        if (readStringOpt(ini, "logSetup.fileLogConf.fileName", sval))
            conf.logSetup.fileLogConf.fileName = sval;
        if (readIntOpt(ini, "logSetup.fileLogConf.fileMaxSize", ival))
            conf.logSetup.fileLogConf.fileMaxSize = static_cast<LSIZE>(ival);
        if (readStringOpt(ini, "logSetup.fileLogConf.logDir", sval))
            conf.logSetup.fileLogConf.logDir = sval;
        if (readStringOpt(ini, "logSetup.fileLogConf.fileNameFmt", sval))
            conf.logSetup.fileLogConf.fileNameFmt = sval;
        if (readIntOpt(ini, "logSetup.fileLogConf.logDirMaxSize", ival))
            conf.logSetup.fileLogConf.logDirMaxSize = static_cast<SIZE>(ival);

        if (readIntOpt(ini, "coreSetup.threadPool", ival))
            conf.coreSetup.threadPool = static_cast<SIZE>(ival);
        if (readIntOpt(ini, "coreSetup.backupThreadMaxTime", ival))
            conf.coreSetup.backupThreadMaxTime = static_cast<uint32_t>(ival);
        if (readStringOpt(ini, "coreSetup.coreSet", sval))
        {
            CoreSet set;
            if (parseCoreSet(sval, set)) conf.coreSetup.coreSet = set;
        }
    }
}  // namespace

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

        case CERBERUS_MESSAGE_TIMEREXPIRY_ID:
            // nothing to add
            break;

            // add here more message specializations..

        default:
            throw cImplMissExc("Requested standard message is not defined");
    }

    return tmplt;
}
//=============================================================================
void Cerberus::registerObj(Recordable* object, const std::string& name)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    Cerberus::framework.core.data->registerObj(object, name);
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
    CerberusInitConf conf = parms;

    IniDataFile ini;
    bool iniLoaded = false;
    if (!conf.appConfigurationFile.empty() || conf.initFromFile)
    {
        ini.setFileName(conf.appConfigurationFile);
        OpRes lr = ini.load();
        if (lr.fail()) throw cFatalExc("application configuration file malformed");
        iniLoaded = true;
        if (conf.initFromFile) applyIniToInitConf(ini, conf);
    }

    Thread::setDefaultCoreSet(conf.coreSetup.coreSet);
    Cerberus::framework.construct(conf);

    if (iniLoaded)
    {
        auto locker = Cerberus::framework.core.getLocker();
        Cerberus::framework.core.data->setIniFile(ini);
        if (!conf.appConfigurationFile.empty())
            Cerberus::framework.core.data->setIniFileName(conf.appConfigurationFile);
    }
    else if (!conf.appConfigurationFile.empty())
    {
        auto locker = Cerberus::framework.core.getLocker();
        Cerberus::framework.core.data->setIniFileName(conf.appConfigurationFile);
    }

    Cerberus::framework.start();

    logDebug("Cerberus init completed");

    if (conf.useCiphers)
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
    toReturn.appConfigurationFile     = "";
    toReturn.initFromFile             = false;

    toReturn.logSetup.fileLogConf.enable        = true;
    toReturn.logSetup.fileLogConf.fileName      = "./app.log";
    toReturn.logSetup.fileLogConf.fileMaxSize   = 4096;
    toReturn.logSetup.fileLogConf.logDir        = "logs";
    toReturn.logSetup.fileLogConf.fileNameFmt   = "%Y-%M-%D_%h-%m-%s.log";
    toReturn.logSetup.fileLogConf.logDirMaxSize = 1000000;

    toReturn.logSetup.appLogLevel  = LL_Error;
    toReturn.logSetup.fwLogLevel = LL_Error;

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
void Cerberus::setFileName(const std::string& fileName)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    Cerberus::framework.core.data->iniFile().setFileName(fileName);
}
//=============================================================================
OpRes Cerberus::load()
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->iniFile().load();
}
//=============================================================================
bool Cerberus::exists(const std::string& key, const std::string& section)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->iniFile().exists(key, section);
}
//=============================================================================
IniDataType Cerberus::type(const std::string& key, const std::string& section)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->iniFile().type(key, section);
}
//=============================================================================
bool Cerberus::isType(const std::string& key, IniDataType type)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->iniFile().isType(key, type);
}
//=============================================================================
OpRes Cerberus::rewrite()
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->iniFile().rewrite();
}
//=============================================================================
OpRes Cerberus::write_string(const std::string& key, const std::string& value, const std::string& section)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->iniFile().write_string(key, value, section);
}
//=============================================================================
OpRes Cerberus::write_integer(const std::string& key, int64_t value, const std::string& section)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->iniFile().write_integer(key, value, section);
}
//=============================================================================
OpRes Cerberus::write_double(const std::string& key, double value, const std::string& section)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->iniFile().write_double(key, value, section);
}
//=============================================================================
OpRes Cerberus::write_bool(const std::string& key, bool value, const std::string& section)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->iniFile().write_bool(key, value, section);
}
//=============================================================================
OpRes Cerberus::enforce_string(const std::string& key, const std::string& value, const std::string& section)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->iniFile().enforce_string(key, value, section);
}
//=============================================================================
OpRes Cerberus::enforce_integer(const std::string& key, int64_t value, const std::string& section)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->iniFile().enforce_integer(key, value, section);
}
//=============================================================================
OpRes Cerberus::enforce_double(const std::string& key, double value, const std::string& section)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->iniFile().enforce_double(key, value, section);
}
//=============================================================================
OpRes Cerberus::enforce_bool(const std::string& key, bool value, const std::string& section)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->iniFile().enforce_bool(key, value, section);
}
//=============================================================================
StringOpRes Cerberus::read_string(const std::string& key, const std::string& section)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->iniFile().read_string(key, section);
}
//=============================================================================
IntOpRes Cerberus::read_integer(const std::string& key, const std::string& section)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->iniFile().read_integer(key, section);
}
//=============================================================================
FloatOpRes Cerberus::read_double(const std::string& key, const std::string& section)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->iniFile().read_double(key, section);
}
//=============================================================================
BoolOpRes Cerberus::read_bool(const std::string& key, const std::string& section)
{
    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();
    return Cerberus::framework.core.data->iniFile().read_bool(key, section);
}
//=============================================================================
OpRes Cerberus::send_deep(const msg_ptr& message, HASH32 recipientID)
{
    if (!message) return OR_WrongArgument;

    if (recipientID != CERBERUS_INVALID_ID) message->setRecipient(recipientID);

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

    message->setRecipient(Cerberus::framework.core.data->objIdByName(recipient));
    return Cerberus::framework.core.data->send_deep(message, idByName(recipient));
}
//=============================================================================
OpRes Cerberus::send_deep(const msg_ptr& message, HASH32 recipientID, HASH32 channel_in)
{
    if (!message) return OR_WrongArgument;

    if (recipientID != CERBERUS_INVALID_ID) message->setRecipient(recipientID);

    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();

    return Cerberus::framework.core.data->sendMsgToObj_deep(recipientID, message, channel_in);
}
//=============================================================================
OpRes Cerberus::send_deep(const msg_ptr& message, const std::string& recipient, HASH32 channel_in)
{
    if (!message) return OR_WrongArgument;

    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();

    message->setRecipient(Cerberus::framework.core.data->objIdByName(recipient));
    return Cerberus::framework.core.data->sendMsgToObj_deep(recipient, message, channel_in);
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

    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();

    message->setRecipient(Cerberus::framework.core.data->objIdByName(recipient));
    return Cerberus::framework.core.data->send(message);
}
//=============================================================================
OpRes Cerberus::send(msg_ptr& message, HASH32 recipientID, HASH32 channel_in)
{
    if (!message) return OR_WrongArgument;

    if (recipientID != CERBERUS_INVALID_ID) message->setRecipient(recipientID);

    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();

    return Cerberus::framework.core.data->sendMsgToObj(recipientID, message, channel_in);
}
//=============================================================================
OpRes Cerberus::send(msg_ptr& message, const std::string& recipient, HASH32 channel_in)
{
    if (!message) return OR_WrongArgument;

    Cerberus::framework.core.isReadySevere();
    auto locker = Cerberus::framework.core.getLocker();

    message->setRecipient(Cerberus::framework.core.data->objIdByName(recipient));
    return Cerberus::framework.core.data->sendMsgToObj(recipient, message, channel_in);
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
