#include "cerberus.h"
#include <cstring>
#include <cstdarg>
#include <iostream>
#include "./exception/exceptioncatalog.h"
#include "./mutex/mutexlocker.h"

#include "./message/slot/charslot.h"

#ifdef WINDOWS_SYSTEM
    // noop
#else
    #include <unistd.h>
#endif

using namespace cerberus;

//=============================================================================
Cerberus::Cerberus() :
    m_useColorInTerminal(false),
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
        stderrPrint("Cerberus already initted, skipping init() call..");
        return;
    }

    // do the initialization:

    if(parms.terminalFormattingDisabled)
    {
        m_useColorInTerminal = false;
    }
    else
    {
        m_useColorInTerminal = _isColorSupported();
    }

    if(m_useColorInTerminal)
    {
        m_stdout_terminalFormatting = _parseFormattingData(parms.stdout_formatting);
        m_stderr_terminalFormatting = _parseFormattingData(parms.stderr_formatting);
    }

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
        throw cerberusIllegalArgumentExc("Given message name is already registered");
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
uint32_t Cerberus::messageIdByName(const std::string& name) const
{
    return m_register.messageIdByName(name);
}
//=============================================================================
message::cerberus_message Cerberus::messageConstruct(uint32_t id) const
{
    message::MessageTemplate found = m_register.messageTemplateById(id);
    message::cerberus_message message = message::Message::create();
    message->setId(id);

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
std::string Cerberus::_parseFormattingData(const TerminalFormatting& data)
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

    throw cerberusIllegalArgumentExc("Factory given ID does not exist or is not yet implemented");
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
void Cerberus::log(const std::string& str, LogLevel logLevel)
{
    mutex::MutexLocker locker(&m_logMutex);

    switch(logLevel)
    {
        case LL_Info:       //writes on stdout
            break;

        case LL_Warning:    //writes on stdout
            break;

        case LL_Error:      //writes on stderr
            break;

        case LL_Debug:      //writes on stdout
            break;
    }
}
//=============================================================================
void Cerberus::stdoutPrint(const std::string& str)
{
    static Cerberus* cerberus = Cerberus::provider();

    if(cerberus->m_useColorInTerminal)
    {
        std::cout << Cerberus::strPrint("%s%s\033[0m", cerberus->m_stdout_terminalFormatting.c_str(), str.c_str()) << std::endl;
    }
    else
    {
        std::cout << str.c_str() << std::endl;
    }
}
//=============================================================================
void Cerberus::stderrPrint(const std::string& str)
{
    static Cerberus* cerberus = Cerberus::provider();

    if(cerberus->m_useColorInTerminal)
    {
        std::cerr << Cerberus::strPrint("%s%s\033[0m", cerberus->m_stderr_terminalFormatting.c_str(), str.c_str()) << std::endl;
    }
    else
    {
        std::cerr << str.c_str() << std::endl;
    }
}
//=============================================================================
