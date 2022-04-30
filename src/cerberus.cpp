#include "cerberus.h"
#include <cstring>
#include <cstdarg>
#include <iostream>
#include "./exception/exceptioncatalog.h"

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
