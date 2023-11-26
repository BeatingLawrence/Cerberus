#include "cerberuslog.h"

#include <iostream>

#include "../cerberus.h"
#include "../message/slot/stringslot.h"
#include "../mutex/mutexlocker.h"
#include "./cerberusfactory.h"
#include "./cerberusutils.h"
#include "src/time/datetime.h"
#include "src/types.h"

using namespace cerberus::core;

const char* CerberusLog::EndOfFormatting_Linux     = "\033[0m";
const uint8_t CerberusLog::EndOfFormatting_Windows = TERMINAL_FOREGROUND_BLUE | TERMINAL_FOREGROUND_GREEN | TERMINAL_FOREGROUND_RED;

#define TIMESTAMP_STRING_LEN 23
#define DEBUG_TAG_STRING_LEN 9  // (including spaces)
#define INFO_TAG_STRING_LEN 8
#define WARNING_TAG_STRING_LEN 11
#define ERROR_TAG_STRING_LEN 9

#ifdef WINDOWS_SYSTEM
#include <windows.h>
#else
#include <unistd.h>
#endif

//=============================================================================
CerberusLog::CerberusLog()
    : m_useFormattedTerminal(false),
      m_fileLogEnable(true),
      m_logLevel(LL_Error),
      m_infoLogTerminalFormatting_Linux(),
      m_warningLogTerminalFormatting_Linux(),
      m_errorLogTerminalFormatting_Linux(),
      m_debugLogTerminalFormatting_Linux(),
      m_stdoutHandle_Windows(nullptr),
      m_stderrHandle_Windows(nullptr),
      m_infoLogTerminalFormatting_Windows(0),
      m_warningLogTerminalFormatting_Windows(0),
      m_errorLogTerminalFormatting_Windows(0),
      m_debugLogTerminalFormatting_Windows(0)
{
    // noop
}
//=============================================================================
CerberusLog::~CerberusLog()
{
    // noop
}
//=============================================================================
std::string CerberusLog::_parseFormattingData_Linux(const CerberusLogRole& data)
{
    std::string toReturn;
    toReturn = "\033[";

    for (auto& el : data.textFormatting)
    {
        if (el != 0)
        {
            toReturn += CerberusUtils::strPrint("%u;", el);
        }
    }

    if (data.foregroundColor != 0)
    {
        toReturn += CerberusUtils::strPrint("%u;", data.foregroundColor);
    }

    if (data.backgroundColor != 0)
    {
        toReturn += CerberusUtils::strPrint("%u;", data.backgroundColor);
    }

    if (toReturn.back() == ';')
    {
        toReturn.pop_back();
    }

    toReturn += 'm';
    return toReturn;
}
//=============================================================================
uint8_t CerberusLog::_parseFormattingData_Windows(const CerberusLogRole& data)
{
    return data.backgroundColor | data.foregroundColor | data.textFormatting[0] | data.textFormatting[1] | data.textFormatting[2];
}
//=============================================================================
CerberusLog* CerberusLog::_instance()
{
    static CerberusLog instance;
    return &instance;
}
//=============================================================================
void CerberusLog::log(const std::string& str, LogLevel logLevel, const std::string& author)
{
    std::string s         = str;
    CerberusLog* instance = _instance();
    mutex::MutexLocker locker(&(instance->m_mutex));

    // loglevel check
    if (logLevel > instance->m_logLevel)
    {
        return;
    }

    // author
    std::string logAuthor;

    if (!author.empty())
    {
        logAuthor = '[';
        logAuthor += author;
        logAuthor += "] ";
    }

    // multiline
    if (isMultiLine(s))
    {
        align(s, logLevel, logAuthor.size());
    }

    std::string rawLog;
    std::string timestamp = time::DateTime::current().toTimeStampString();

    switch (logLevel)
    {
        case LL_Info:
            rawLog = CerberusUtils::strPrint("%s [INFO] %s%s", timestamp.c_str(), logAuthor.c_str(), s.c_str());
            break;

        case LL_Warning:
            rawLog = CerberusUtils::strPrint("%s [WARNING] %s%s", timestamp.c_str(), logAuthor.c_str(), s.c_str());
            break;

        case LL_Error:
            rawLog = CerberusUtils::strPrint("%s [ERROR] %s%s", timestamp.c_str(), logAuthor.c_str(), s.c_str());
            break;

        case LL_Debug:
            rawLog = CerberusUtils::strPrint("%s [DEBUG] %s%s", timestamp.c_str(), logAuthor.c_str(), s.c_str());
            break;
    }

    if (instance->m_fileLogEnable)
    {
        // Log on file
        message::cerberus_message logMessage = core::CerberusFactory::standardMessageConstruct(SM_LogMsg);
        logMessage->getSlotAt(0)->to<message::slot::StringSlot>()->setValue(rawLog);
        Cerberus::send(logMessage);
    }

    if (!(instance->m_useFormattedTerminal))
    {
        std::cout << rawLog.c_str() << std::endl;
        return;
    }

#if defined(LINUX_SYSTEM) || defined(APPLE_SYSTEM)

    switch (logLevel)
    {
        case LL_Info:  // writes on stdout
            std::cout << CerberusUtils::strPrint("%s%s [%sINFO%s] %s%s", EndOfFormatting_Linux, timestamp.c_str(), instance->m_infoLogTerminalFormatting_Linux.c_str(), EndOfFormatting_Linux,
                                                 logAuthor.c_str(), s.c_str())
                      << std::endl;
            break;

        case LL_Warning:  // writes on stdout
            std::cout << CerberusUtils::strPrint("%s%s [%sWARNING%s] %s%s", EndOfFormatting_Linux, timestamp.c_str(), instance->m_warningLogTerminalFormatting_Linux.c_str(), EndOfFormatting_Linux,
                                                 logAuthor.c_str(), s.c_str())
                      << std::endl;
            break;

        case LL_Error:  // writes on stderr
            std::cerr << CerberusUtils::strPrint("%s%s [%sERROR%s] %s%s", EndOfFormatting_Linux, timestamp.c_str(), instance->m_errorLogTerminalFormatting_Linux.c_str(), EndOfFormatting_Linux,
                                                 logAuthor.c_str(), s.c_str())
                      << std::endl;
            break;

        case LL_Debug:  // writes on stderr
            std::cerr << CerberusUtils::strPrint("%s%s [%sDEBUG%s] %s%s", EndOfFormatting_Linux, timestamp.c_str(), instance->m_debugLogTerminalFormatting_Linux.c_str(), EndOfFormatting_Linux,
                                                 logAuthor.c_str(), s.c_str())
                      << std::endl;
            break;
    }

#else

    switch (logLevel)
    {
        case LL_Info:  // writes on stdout
            SetConsoleTextAttribute(instance->m_stdoutHandle_Windows, EndOfFormatting_Windows);
            std::cout << CerberusUtils::strPrint("%s [", timestamp.c_str());
            SetConsoleTextAttribute(instance->m_stdoutHandle_Windows, instance->m_infoLogTerminalFormatting_Windows);
            std::cout << "INFO";
            SetConsoleTextAttribute(instance->m_stdoutHandle_Windows, EndOfFormatting_Windows);
            std::cout << CerberusUtils::strPrint("] %s%s", logAuthor.c_str(), s.c_str());
            std::cout << std::endl;
            break;

        case LL_Warning:  // writes on stdout
            SetConsoleTextAttribute(instance->m_stdoutHandle_Windows, EndOfFormatting_Windows);
            std::cout << CerberusUtils::strPrint("%s [", timestamp.c_str());
            SetConsoleTextAttribute(instance->m_stdoutHandle_Windows, instance->m_warningLogTerminalFormatting_Windows);
            std::cout << "WARNING";
            SetConsoleTextAttribute(instance->m_stdoutHandle_Windows, EndOfFormatting_Windows);
            std::cout << CerberusUtils::strPrint("] %s%s", logAuthor.c_str(), s.c_str());
            std::cout << std::endl;
            break;

        case LL_Error:  // writes on stderr
            SetConsoleTextAttribute(instance->m_stdoutHandle_Windows, EndOfFormatting_Windows);
            std::cerr << CerberusUtils::strPrint("%s [", timestamp.c_str());
            SetConsoleTextAttribute(instance->m_stdoutHandle_Windows, instance->m_errorLogTerminalFormatting_Windows);
            std::cerr << "ERROR";
            SetConsoleTextAttribute(instance->m_stdoutHandle_Windows, EndOfFormatting_Windows);
            std::cerr << CerberusUtils::strPrint("] %s%s", logAuthor.c_str(), s.c_str());
            std::cerr << std::endl;
            break;

        case LL_Debug:  // writes on stderr
            SetConsoleTextAttribute(instance->m_stdoutHandle_Windows, EndOfFormatting_Windows);
            std::cerr << CerberusUtils::strPrint("%s [", timestamp.c_str());
            SetConsoleTextAttribute(instance->m_stdoutHandle_Windows, instance->m_debugLogTerminalFormatting_Windows);
            std::cerr << "DEBUG";
            SetConsoleTextAttribute(instance->m_stdoutHandle_Windows, EndOfFormatting_Windows);
            std::cerr << CerberusUtils::strPrint("] %s%s", logAuthor.c_str(), s.c_str());
            std::cerr << std::endl;
            break;
    }

#endif
}
//=============================================================================
void CerberusLog::llDebug(const std::string& str, const std::string& author)
{
    // time
    std::string timestamp = time::DateTime::current().toTimeStampString();
    // author
    std::string logAuthor;

    if (!author.empty())
    {
        logAuthor = '[';
        logAuthor += author;
        logAuthor += "] ";
    }

    std::cerr << CerberusUtils::strPrint("%s [DEBUG] %s%s", timestamp.c_str(), logAuthor.c_str(), str.c_str()) << std::endl;
}
//=============================================================================
void CerberusLog::_setup(const CerberusLogSetup& setup)
{
    CerberusLog* instance = _instance();
    mutex::MutexLocker locker(&(instance->m_mutex));
    instance->m_stdoutHandle_Windows                 = nullptr;
    instance->m_stderrHandle_Windows                 = nullptr;
    instance->m_infoLogTerminalFormatting_Windows    = 0;
    instance->m_warningLogTerminalFormatting_Windows = 0;
    instance->m_errorLogTerminalFormatting_Windows   = 0;
    instance->m_debugLogTerminalFormatting_Windows   = 0;
    instance->m_infoLogTerminalFormatting_Linux      = std::string();
    instance->m_warningLogTerminalFormatting_Linux   = std::string();
    instance->m_errorLogTerminalFormatting_Linux     = std::string();
    instance->m_debugLogTerminalFormatting_Linux     = std::string();
    instance->m_logLevel                             = setup.logLevel;

    if (setup.disableFormatting)
    {
        instance->m_useFormattedTerminal = false;
    }
    else
    {
#ifdef WINDOWS_SYSTEM
        instance->m_useFormattedTerminal = true;
#else
        instance->m_useFormattedTerminal = (isatty(fileno(stdout)) == 1);
#endif
    }

    if (instance->m_useFormattedTerminal)
    {
#ifdef WINDOWS_SYSTEM
        instance->m_stdoutHandle_Windows                 = GetStdHandle(STD_OUTPUT_HANDLE);
        instance->m_stderrHandle_Windows                 = GetStdHandle(STD_ERROR_HANDLE);
        instance->m_infoLogTerminalFormatting_Windows    = instance->_parseFormattingData_Windows(setup.infoRole);
        instance->m_warningLogTerminalFormatting_Windows = instance->_parseFormattingData_Windows(setup.warningRole);
        instance->m_errorLogTerminalFormatting_Windows   = instance->_parseFormattingData_Windows(setup.errorRole);
        instance->m_debugLogTerminalFormatting_Windows   = instance->_parseFormattingData_Windows(setup.debugRole);
#else
        instance->m_infoLogTerminalFormatting_Linux    = instance->_parseFormattingData_Linux(setup.infoRole);
        instance->m_warningLogTerminalFormatting_Linux = instance->_parseFormattingData_Linux(setup.warningRole);
        instance->m_errorLogTerminalFormatting_Linux   = instance->_parseFormattingData_Linux(setup.errorRole);
        instance->m_debugLogTerminalFormatting_Linux   = instance->_parseFormattingData_Linux(setup.debugRole);
#endif
    }
}
//=============================================================================
bool CerberusLog::isMultiLine(const std::string& str) { return CerberusUtils::contains(str, "\n"); }
//=============================================================================
void CerberusLog::align(std::string& str, LogLevel logLevel, uint32_t authorLen)
{
    uint32_t indentation = TIMESTAMP_STRING_LEN;

    switch (logLevel)
    {
        case LL_Info:
            indentation += INFO_TAG_STRING_LEN;
            break;
        case LL_Warning:
            indentation += WARNING_TAG_STRING_LEN;
            break;
        case LL_Error:
            indentation += ERROR_TAG_STRING_LEN;
            break;
        case LL_Debug:
            indentation += DEBUG_TAG_STRING_LEN;
            break;
    }

    indentation += authorLen;
    std::string indentStr(indentation, ' ');
    CerberusUtils::replaceAll(str, "\n", CerberusUtils::strPrint("\n%s", indentStr.c_str()));
    str = CerberusUtils::removeBlankAfter(str);
}
//=============================================================================
