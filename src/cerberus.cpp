#include "cerberus.h"
#include <cstring>
#include <cstdarg>
#include <iostream>
#include "./exception/exceptioncatalog.h"
#include "./mutex/mutexlocker.h"
#include "./thread/thread.h"
#include "./message/messagetemplate.h"
#include "./message/slot/charslot.h"
#include <chrono>
#include <ctime>

#ifdef WINDOWS_SYSTEM
    // TBD
#else
    #include <unistd.h>
#endif

using namespace cerberus;
const char* Cerberus::EndOfFormatting = "\033[0m";

//=============================================================================
Cerberus::Cerberus() :
    m_useFormattedTerminal(false),
    m_initFlag(false),
    m_coreThread(nullptr)
{
    // noop
}
//=============================================================================
Cerberus* Cerberus::_provider()
{
    static Cerberus cerberus;
    return &cerberus;
}
//=============================================================================
Cerberus::~Cerberus()
{
    m_coreThread->join();
    delete m_coreThread;
    logInfo("Cerberus Memory Released");
}
//=============================================================================
void Cerberus::init(const CerberusInitParms& parms)
{
    Cerberus* cerberus = _provider();

    if(cerberus->m_initFlag)
    {
        log("Cerberus already initted, skipping init() call..");
        return;
    }

    // do the initialization:

    if(parms.terminalFormattingDisabled)
    {
        cerberus->m_useFormattedTerminal = false;
    }
    else
    {
        cerberus->m_useFormattedTerminal = cerberus->_isColorSupported();
    }

    if(cerberus->m_useFormattedTerminal)
    {
        cerberus->m_infoLogTerminalFormatting = cerberus->_parseFormattingData(parms.terminal.infoRole);
        cerberus->m_warningLogTerminalFormatting = cerberus->_parseFormattingData(parms.terminal.warningRole);
        cerberus->m_errorLogTerminalFormatting = cerberus->_parseFormattingData(parms.terminal.errorRole);
        cerberus->m_debugLogTerminalFormatting = cerberus->_parseFormattingData(parms.terminal.debugRole);
    }

    cerberus->m_coreThread = new thread::Thread("Core Thread");
    cerberus->m_coreThread->provideWarmUpCallback(&coreWarmUp);
    cerberus->m_coreThread->provideCoolDownCallback(&coreCoolDown);
    cerberus->m_coreThread->provideTickCallback(&coreTick);
    cerberus->m_coreThread->start();
    //Do other stuff..
    cerberus->m_initFlag = true;
    logInfo("Cerberus init completed");
}
//=============================================================================
uint32_t Cerberus::messageTypeIdByName(const std::string& name)
{
    Cerberus* cerberus = _provider();
    CerberusObject* found = cerberus->m_register.cerberusObjectByName(name);

    if(found == nullptr)
    {
        return CERBERUS_INVALID_ID;
    }
    else
    {
        if(found->type() == CERBERUS_OBJECT_MESSAGETMPLT)
        {
            return found->id();
        }
        else
        {
            return CERBERUS_INVALID_ID;
        }
    }
}
//=============================================================================
message::cerberus_message Cerberus::messageConstruct(uint32_t typeID)
{
    Cerberus* cerberus = _provider();
    CerberusObject* found = cerberus->m_register.cerberusObjectByID(typeID);

    if(found == nullptr)
    {
        throw cerberusIllegalArgumentExc("Factory given ID does not exist");
    }

    if(found->type() != CERBERUS_OBJECT_MESSAGETMPLT)
    {
        throw cerberusIllegalArgumentExc("Factory given ID is not a message ID");
    }

    message::MessageTemplate* tmplt = found->to<message::MessageTemplate>();
    message::cerberus_message message = message::Message::create(found->id());

    for(size_t i = 0; i < tmplt->count(); i++)
    {
        message->addSlot(_slotFactory(tmplt->getSlotTypeAt(i)));
    }

    return message;
}
//=============================================================================
void Cerberus::send(message::cerberus_message message)
{
    _provider()->m_coreThread->addMessage(message);
}
//=============================================================================
uint32_t Cerberus::threadIdByName(const std::string& name)
{
    Cerberus* cerberus = _provider();
    CerberusObject* found = cerberus->m_register.cerberusObjectByName(name);

    if(found == nullptr)
    {
        return CERBERUS_INVALID_ID;
    }
    else
    {
        if(found->type() == CERBERUS_OBJECT_THREAD)
        {
            return found->id();
        }
        else
        {
            return CERBERUS_INVALID_ID;
        }
    }
}
//=============================================================================
uint32_t Cerberus::_registerCerberusObject(CerberusObject* object)
{
    return m_register.registerCerberusObject(object);
}
//=============================================================================
void Cerberus::_unregisterCerberusObject(uint32_t id)
{
    m_register.unregisterCerberusObject(id);
}
//=============================================================================
bool Cerberus::_isColorSupported()
{
    bool colorSupported = false;
#ifdef WINDOWS_SYSTEM
    colorSupported = false; //TODO add windows terminal management
#else

    if(isatty(fileno(stdout)) == 1)
    {
        colorSupported = true;
    }

#endif
    return colorSupported;
}
//=============================================================================
std::string Cerberus::_parseFormattingData(const CerberusCustomizedLogRole& data)
{
    std::string toReturn;
    toReturn = "\033[";

    for(auto& el : data.textFormatting)
    {
        if(el != 0)
        {
            toReturn += Cerberus::strPrint("%u;", el);
        }
    }

    if(data.foregroundColor != 0)
    {
        toReturn += Cerberus::strPrint("%u;", data.foregroundColor);
    }

    if(data.backgroundColor != 0)
    {
        toReturn += Cerberus::strPrint("%u;", data.backgroundColor);
    }

    if(toReturn.back() == ';')
    {
        toReturn.pop_back();
    }

    toReturn += 'm';
    return toReturn;
}
//=============================================================================
message::slot::cerberus_slot Cerberus::_slotFactory(message::slot::SlotType type)
{
    switch(type)
    {
        case message::slot::ST_UCHAR:
            // TODO to implement
            break;

        case message::slot::ST_CHAR:
            return message::slot::CharSlot::create();
            break;

        case message::slot::ST_USHORT:
            // TODO to implement
            break;

        case message::slot::ST_SHORT:
            // TODO to implement
            break;

        case message::slot::ST_ULONG:
            // TODO to implement
            break;

        case message::slot::ST_LONG:
            // TODO to implement
            break;

        case message::slot::ST_ULONGLONG:
            // TODO to implement
            break;

        case message::slot::ST_LONGLONG:
            // TODO to implement
            break;

        case message::slot::ST_FLOAT:
            // TODO to implement
            break;

        case message::slot::ST_DOUBLE:
            // TODO to implement
            break;

        case message::slot::ST_BOOL:
            // TODO to implement
            break;
    }

    throw cerberusIllegalArgumentExc("SlotFactory: Given slot type does not exist");
}
//=============================================================================
void Cerberus::coreWarmUp()
{
    logInfo("Starting Core Thread..");
    //Register message specializations
    message::Message shutdownMessage;
    registerMessage(shutdownMessage, "ShutdownMessage");
}
//=============================================================================
void Cerberus::coreCoolDown()
{
    logInfo("Stopping Cerberus Core..");
    Cerberus* cerberus = _provider();

    if(!(cerberus->m_register.isEmpty()))
    {
        logWarning("Trying to free register memory");
        cerberus->m_register.freeMemory();
    }
}
//=============================================================================
int Cerberus::coreTick(message::cerberus_message message, thread::Thread* thread)
{
    if(message->isValid())
    {
        //Process message queue..
        uint32_t destination = message->destinationID();

        if(destination == CERBERUS_INVALID_ID)
        {
            logInfo("Destination of message is invalid, dropping..");
        }
        else
        {
            Cerberus* cerberus = _provider();
            CerberusObject* found =  cerberus->m_register.cerberusObjectByID(destination);

            if(found == nullptr)
            {
                logInfo("Destination of message is unknown, dropping..");
            }
            else
            {
                if(found->type() == CERBERUS_OBJECT_THREAD)
                {
                    thread::Thread* thread = found->to<thread::Thread>();
                    thread->addMessage(message);    //ownership transferred
                }
                else
                {
                    logInfo("Destination of message cannot accept messages, dropping..");
                }

                //ADD other messages receivers here..
            }
        }
    }

    //Do other stuff..
    //TODO: Log on file
    return 0;
}
//=============================================================================
std::string Cerberus::strPrint(const char* format, ...)
{
    std::string ret;

    if(format != nullptr)
    {
        if(strlen(format) != 0)
        {
            va_list testList;
            va_list list;
            va_start(testList, format);
            va_copy(list, testList);
            char garbage;
            int required = vsnprintf(&garbage, 0, format, testList);

            if(required > 0)
            {
                ret.resize(required);
                vsnprintf(&ret[0], required + 1, format, list);
            }

            va_end(testList);
            va_end(list);
        }
    }

    return ret;
}
//=============================================================================
CerberusInitParms Cerberus::cerberusDefaultParms()
{
    CerberusInitParms toReturn = {};
    toReturn.terminalFormattingDisabled = false;
    toReturn.terminal.infoRole.backgroundColor = TERMINAL_BACKGROUND_BLACK;
    toReturn.terminal.warningRole.backgroundColor = TERMINAL_BACKGROUND_BLACK;
    toReturn.terminal.errorRole.backgroundColor = TERMINAL_BACKGROUND_BLACK;
    toReturn.terminal.debugRole.backgroundColor = TERMINAL_BACKGROUND_BLACK;
    toReturn.terminal.infoRole.foregroundColor = TERMINAL_FOREGROUND_GREEN;
    toReturn.terminal.warningRole.foregroundColor = TERMINAL_FOREGROUND_YELLOW;
    toReturn.terminal.errorRole.foregroundColor = TERMINAL_FOREGROUND_RED;
    toReturn.terminal.debugRole.foregroundColor = TERMINAL_FOREGROUND_MAGENTA;
    return toReturn;
}
//=============================================================================
void Cerberus::log(const std::string& str, LogLevel logLevel, const std::string& author)
{
    static mutex::Mutex mutex;
    mutex::MutexLocker locker(&mutex);
    Cerberus* cerberus = Cerberus::_provider();
    //time
    auto now = std::chrono::system_clock::now();
    auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
    auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(now - seconds);
    time_t coarseTime = std::chrono::system_clock::to_time_t(now);
    tm* local = std::localtime(&coarseTime);
    uint32_t fineTime = milli.count();
    std::string timestamp = strPrint("%.4u.%.2u.%.2u-%.2u:%.2u:%.2u.%.3u", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec, fineTime);
    std::string logAuthor;

    if(!author.empty())
    {
        logAuthor = '[';
        logAuthor += author;
        logAuthor += "] ";
    }

    //
    switch(logLevel)
    {
        case LL_Info:       //writes on stdout
            if(cerberus->m_useFormattedTerminal)
            {
                std::cout << strPrint("%s%s [%sINFO%s] %s%s",
                                      EndOfFormatting,
                                      timestamp.c_str(),
                                      cerberus->m_infoLogTerminalFormatting.c_str(),
                                      EndOfFormatting,
                                      logAuthor.c_str(),
                                      str.c_str()) << std::endl;
            }
            else
            {
                std::cout << strPrint("%s [INFO] %s%s", timestamp.c_str(), logAuthor.c_str(), str.c_str()) << std::endl;
            }

            break;

        case LL_Warning:    //writes on stdout
            if(cerberus->m_useFormattedTerminal)
            {
                std::cout << strPrint("%s%s [%sWARNING%s] %s%s",
                                      EndOfFormatting,
                                      timestamp.c_str(),
                                      cerberus->m_warningLogTerminalFormatting.c_str(),
                                      EndOfFormatting,
                                      logAuthor.c_str(),
                                      str.c_str()) << std::endl;
            }
            else
            {
                std::cout << strPrint("%s [WARNING] %s%s", timestamp.c_str(), logAuthor.c_str(), str.c_str()) << std::endl;
            }

            break;

        case LL_Error:      //writes on stderr
            if(cerberus->m_useFormattedTerminal)
            {
                std::cerr << strPrint("%s%s [%sERROR%s] %s%s",
                                      EndOfFormatting,
                                      timestamp.c_str(),
                                      cerberus->m_errorLogTerminalFormatting.c_str(),
                                      EndOfFormatting,
                                      logAuthor.c_str(),
                                      str.c_str()) << std::endl;
            }
            else
            {
                std::cerr << strPrint("%s [ERROR] %s%s", timestamp.c_str(), logAuthor.c_str(), str.c_str()) << std::endl;
            }

            break;

        case LL_Debug:      //writes on stdout
            if(cerberus->m_useFormattedTerminal)
            {
                std::cout << strPrint("%s%s [%sDEBUG%s] %s%s",
                                      EndOfFormatting,
                                      timestamp.c_str(),
                                      cerberus->m_debugLogTerminalFormatting.c_str(),
                                      EndOfFormatting,
                                      logAuthor.c_str(),
                                      str.c_str()) << std::endl;
            }
            else
            {
                std::cout << strPrint("%s [DEBUG] %s%s", timestamp.c_str(), logAuthor.c_str(), str.c_str()) << std::endl;
            }

            break;
    }
}
//=============================================================================
uint32_t Cerberus::registerMessage(const message::Message& message, const std::string& name)
{
    return (new cerberus::message::MessageTemplate(message, name))->id();
}
//=============================================================================
