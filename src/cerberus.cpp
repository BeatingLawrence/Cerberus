#include "cerberus.h"
#include <cstring>
#include <cstdarg>
#include <iostream>
#include "./exception/exceptioncatalog.h"
#include "./mutex/mutexlocker.h"
#include <chrono>
#include <ctime>

#include "./message/slot/charslot.h"

#ifdef WINDOWS_SYSTEM
    // noop
#else
    #include <unistd.h>
#endif

using namespace cerberus;
const char* Cerberus::EndOfFormatting = "\033[0m";

//=============================================================================
Cerberus::Cerberus() :
    m_useFormattedTerminal(false),
    m_initFlag(false)
{
    // noop
}
//=============================================================================
Cerberus* Cerberus::provider()
{
    static Cerberus cerberus;
    return &cerberus;
}
//=============================================================================
void Cerberus::init(const CerberusInitParms& parms)
{
    if(m_initFlag)
    {
        log("Cerberus already initted, skipping init() call..");
        return;
    }

    // do the initialization:

    if(parms.terminalFormattingDisabled)
    {
        m_useFormattedTerminal = false;
    }
    else
    {
        m_useFormattedTerminal = _isColorSupported();
    }

    if(m_useFormattedTerminal)
    {
        m_infoLogTerminalFormatting = _parseFormattingData(parms.terminal.infoRole);
        m_warningLogTerminalFormatting = _parseFormattingData(parms.terminal.warningRole);
        m_errorLogTerminalFormatting = _parseFormattingData(parms.terminal.errorRole);
        m_debugLogTerminalFormatting = _parseFormattingData(parms.terminal.debugRole);
        std::cout << m_infoLogTerminalFormatting << std::endl;
        std::cout << m_warningLogTerminalFormatting << std::endl;
    }

    //Do other stuff..
    m_initFlag = true;
}
//=============================================================================
uint32_t Cerberus::registerMessage(const message::Message& message, const std::string& name)
{
    if(message.count() == 0)
    {
        throw cerberusIllegalArgumentExc("Cannot register an empty message");
    }

    if(m_register.messageTemplateNameAlreadyExists(name))
    {
        throw cerberusIllegalArgumentExc("Given Message name is already registered");
    }

    message::MessageTemplate tmplt(message, name);
    return m_register.addMessageTemplate(tmplt);
}
//=============================================================================
void Cerberus::forgetMessage(uint32_t id)
{
    m_register.removeMessageTemplate(id);
}
//=============================================================================
uint32_t Cerberus::messageTypeIdByName(const std::string& name) const
{
    return m_register.messageTypeIdByName(name);
}
//=============================================================================
message::cerberus_message Cerberus::messageConstruct(uint32_t typeID) const
{
    message::MessageTemplate found = m_register.messageTemplateByTypeId(typeID);
    message::cerberus_message message = message::Message::create();
    message->setId(typeID);

    for(size_t i = 0; i < found.count(); i++)
    {
        message::slot::cerberus_slot slot = _newSlot(found.getSlotTypeAt(i));
        message->addSlot(slot);
    }

    return message;
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
message::slot::cerberus_slot Cerberus::_newSlot(message::slot::BaseSlot::SlotType type)
{
    switch(type)
    {
        case message::slot::BaseSlot::ST_UCHAR:
            return message::slot::CharSlot::create();
            break;

        case message::slot::BaseSlot::ST_CHAR:
            // TODO to implement
            break;

        case message::slot::BaseSlot::ST_USHORT:
            // TODO to implement
            break;

        case message::slot::BaseSlot::ST_SHORT:
            // TODO to implement
            break;

        case message::slot::BaseSlot::ST_ULONG:
            // TODO to implement
            break;

        case message::slot::BaseSlot::ST_LONG:
            // TODO to implement
            break;

        case message::slot::BaseSlot::ST_ULONGLONG:
            // TODO to implement
            break;

        case message::slot::BaseSlot::ST_LONGLONG:
            // TODO to implement
            break;

        case message::slot::BaseSlot::ST_FLOAT:
            // TODO to implement
            break;

        case message::slot::BaseSlot::ST_DOUBLE:
            // TODO to implement
            break;

        case message::slot::BaseSlot::ST_BOOL:
            // TODO to implement
            break;
    }

    throw cerberusIllegalArgumentExc("Factory given type does not exist or is not implemented yet");
}
//=============================================================================
uint32_t Cerberus::_registerThread(thread::Thread* thread, const std::string& name)
{
    if(thread == nullptr)
    {
        throw cerberusIllegalArgumentExc("Cannot register a null Thread");
    }

    if(m_register.threadNameAlreadyExists(name))
    {
        throw cerberusIllegalArgumentExc("Given Thread name is already registered");
    }

    return m_register.addThread(thread, name);
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
void Cerberus::log(const std::string& str, LogLevel logLevel)
{
    static mutex::Mutex mutex;
    mutex::MutexLocker locker(&mutex);
    static Cerberus* cerberus = Cerberus::provider();
    //time
    auto now = std::chrono::system_clock::now();
    auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
    auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(now - seconds);
    time_t coarseTime = std::chrono::system_clock::to_time_t(now);
    tm* local = std::localtime(&coarseTime);
    uint32_t fineTime = milli.count();
    std::string timestamp = strPrint("%.4u.%.2u.%.2u-%.2u:%.2u:%.2u.%.3u", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec, fineTime);

    //
    switch(logLevel)
    {
        case LL_Info:       //writes on stdout
            if(cerberus->m_useFormattedTerminal)
            {
                std::cout << strPrint("%s [%sINFO%s] %s", timestamp.c_str(), cerberus->m_infoLogTerminalFormatting.c_str(), EndOfFormatting, str.c_str()) << std::endl;
            }
            else
            {
                std::cout << strPrint("%s [INFO] %s", timestamp.c_str(), str.c_str()) << std::endl;
            }

            break;

        case LL_Warning:    //writes on stdout
            if(cerberus->m_useFormattedTerminal)
            {
                std::cout << strPrint("%s [%sWARNING%s] %s", timestamp.c_str(), cerberus->m_warningLogTerminalFormatting.c_str(), EndOfFormatting, str.c_str()) << std::endl;
            }
            else
            {
                std::cout << strPrint("%s [WARNING] %s", timestamp.c_str(), str.c_str()) << std::endl;
            }

            break;

        case LL_Error:      //writes on stderr
            if(cerberus->m_useFormattedTerminal)
            {
                std::cerr << strPrint("%s [%sERROR%s] %s", timestamp.c_str(), cerberus->m_errorLogTerminalFormatting.c_str(), EndOfFormatting, str.c_str()) << std::endl;
            }
            else
            {
                std::cerr << strPrint("%s [ERROR] %s", timestamp.c_str(), str.c_str()) << std::endl;
            }

            break;

        case LL_Debug:      //writes on stdout
            if(cerberus->m_useFormattedTerminal)
            {
                std::cout << strPrint("%s [%sDEBUG%s] %s", timestamp.c_str(), cerberus->m_debugLogTerminalFormatting.c_str(), EndOfFormatting, str.c_str()) << std::endl;
            }
            else
            {
                std::cout << strPrint("%s [DEBUG] %s", timestamp.c_str(), str.c_str()) << std::endl;
            }

            break;
    }
}
//=============================================================================
