#include "cerberuslog.h"

#include <stdio.h>

#include "../cerberus.h"
#include "../time/datetime.h"
#include "../types.h"
#include "./cerberusutils.h"

using namespace cerberus::core;

#define TIMESTAMP_STRING_LEN 23
#define DEBUG_TAG_STRING_LEN 9  // (including spaces)
#define INFO_TAG_STRING_LEN 8
#define WARNING_TAG_STRING_LEN 11
#define ERROR_TAG_STRING_LEN 9
#define CERB_TAG_STR "[CERB]"

#ifdef WINDOWS_SYSTEM
#include <windows.h>
#define NEWLINE "\r\n"
const uint8_t CerberusLog::EndOfFormatting_Windows =
    TERMINAL_FOREGROUND_BLUE | TERMINAL_FOREGROUND_GREEN | TERMINAL_FOREGROUND_RED;
#else
#include <unistd.h>
#define NEWLINE "\n"
const char* CerberusLog::m_endForm = "\033[0m";
#endif

//=============================================================================
CerberusLog::CerberusLog()
    : m_logConf(),
      m_logger(nullptr),
      m_loggerFlag(false)
{
    m_logConf = Cerberus::cerberusDefaultParms().logSetup;
}
//=============================================================================
CerberusLog::~CerberusLog()
{
    if (!m_logger) return;

    m_logger->join(true);
    delete m_logger;
}
//=============================================================================
#if defined LINUX_SYSTEM || defined APPLE_SYSTEM
std::string CerberusLog::parseFormdata(const LogRole& data)
{
    std::string toReturn;
    toReturn = "\033[";

    for (auto& el : data.textFormatting)
        if (el != 0) toReturn += CerberusUtils::strPrint("%u;", el);

    if (data.foregroundColor != 0) toReturn += CerberusUtils::strPrint("%u;", data.foregroundColor);

    if (data.backgroundColor != 0) toReturn += CerberusUtils::strPrint("%u;", data.backgroundColor);

    if (toReturn.back() == ';') toReturn.pop_back();

    toReturn += 'm';
    return toReturn;
}
#endif
//=============================================================================
#ifdef WINDOWS_SYSTEM
uint8_t CerberusLog::parseFormattingData_Windows(const LogRole& data)
{
    return data.backgroundColor | data.foregroundColor | data.textFormatting[0] | data.textFormatting[1] |
           data.textFormatting[2];
}
#endif
//=============================================================================
void CerberusLog::log(const std::string& str, LogLevel logLevel, const std::string& author, bool application)
{
    // loglevel check
    if ((application && logLevel > m_logConf.appLogLevel) ||
        (!application && logLevel > m_logConf.cerbLogLevel))
        return;

    // author
    std::string logAuth = authStr(author, application);

    // time
    std::string time = DateTime::current().toTimeStampString();

    // multiline check
    std::string s = str;
    if (isMultiLine(s)) align(s, logLevel, logAuth.size());

    // rawLog (no formatting)
    std::string rawLog;
    if (rawLogNeeded()) rawLog = toRawLog(s, logLevel, logAuth, time);

    // log on terminal
    if (m_logConf.colorFormatting)
    {
        sysPrint(stream(logLevel), toFormattedLog(s, logLevel, logAuth, time));
    }
    else
    {
        sysPrint(stream(logLevel), rawLog);
    }

    rawLog.pop_back();  // remove the \n

    // file log
    if (fileLoggerAvail())
    {
        msg_ptr logMessage = Cerberus::constructMessage(CERBERUS_MESSAGE_LOG_ID);
        logMessage->getSlotAt(0)->to<StringSlot>()->value(rawLog);
        m_logger->addMessage(std::move(logMessage));
    }
}
//=============================================================================
void CerberusLog::llDebug(const std::string& str)
{
    sysPrint(stream(LL_Debug), toRawLog(str, LL_Debug, "", DateTime::current().toTimeStampString()));
}
//=============================================================================
void CerberusLog::setup(const LogConf& parms)
{
    m_logConf = parms;
    // m_stdoutHandle_Windows                 = nullptr;
    // m_stderrHandle_Windows                 = nullptr;
    // m_infoLogTerminalFormatting_Windows    = 0;
    // m_warningLogTerminalFormatting_Windows = 0;
    // m_errorLogTerminalFormatting_Windows   = 0;
    // m_debugLogTerminalFormatting_Windows   = 0;
    m_infoForm.clear();
    m_warnForm.clear();
    m_errForm.clear();
    m_debForm.clear();

#ifndef WINDOWS_SYSTEM
    if (m_logConf.colorFormatting)
    {
        // determine if wanted color formatting is actually applicable
        m_logConf.colorFormatting = (isatty(fileno(stdout)) == 1);
    }
#endif

    if (m_logConf.colorFormatting)
    {
#ifdef WINDOWS_SYSTEM
        m_stdoutHandle_Windows                 = GetStdHandle(STD_OUTPUT_HANDLE);
        m_stderrHandle_Windows                 = GetStdHandle(STD_ERROR_HANDLE);
        m_infoLogTerminalFormatting_Windows    = parseFormattingData_Windows(setup.infoRole);
        m_warningLogTerminalFormatting_Windows = parseFormattingData_Windows(setup.warningRole);
        m_errorLogTerminalFormatting_Windows   = parseFormattingData_Windows(setup.errorRole);
        m_debugLogTerminalFormatting_Windows   = parseFormattingData_Windows(setup.debugRole);
#else
        m_infoForm = parseFormdata(parms.infoRole);
        m_warnForm = parseFormdata(parms.warningRole);
        m_errForm  = parseFormdata(parms.errorRole);
        m_debForm  = parseFormdata(parms.debugRole);
#endif
    }
}
//=============================================================================
void CerberusLog::start()
{
    if (!m_logConf.fileLogConf.enable) return;

    m_logger = new LoggerThread;
    m_logger->setup(m_logConf.fileLogConf);
    m_logger->start();
    m_loggerFlag.store(true);
}
//=============================================================================
void CerberusLog::stop()
{
    if (!m_loggerFlag.load()) return;

    m_loggerFlag.store(false);

    m_logger->join(true);
    delete m_logger;
    m_logger = nullptr;
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
        default:
            break;
    }

    indentation += authorLen;
    std::string indentStr(indentation, ' ');
    CerberusUtils::replaceAll(str, "\n", CerberusUtils::strPrint("\n%s", indentStr.c_str()));
    str = CerberusUtils::removeBlankAfter(str);
}
//=============================================================================
std::string CerberusLog::toRawLog(const std::string& str, LogLevel ll, const std::string& a,
                                  const std::string& t)
{
    switch (ll)
    {
        case LL_Info:
            return CerberusUtils::strPrint("%s [INFO] %s%s" NEWLINE, t.c_str(), a.c_str(), str.c_str());

        case LL_Warning:
            return CerberusUtils::strPrint("%s [WARNING] %s%s" NEWLINE, t.c_str(), a.c_str(), str.c_str());

        case LL_Error:
            return CerberusUtils::strPrint("%s [ERROR] %s%s" NEWLINE, t.c_str(), a.c_str(), str.c_str());

        case LL_Debug:
            return CerberusUtils::strPrint("%s [DEBUG] %s%s" NEWLINE, t.c_str(), a.c_str(), str.c_str());

        default:
            break;
    }

    return "";
}
//=============================================================================
std::string CerberusLog::toFormattedLog(const std::string& str, LogLevel ll, const std::string& a,
                                        const std::string& t)
{
#ifdef WINDOWS_SYSTEM
    // remake this part under windows !!

    switch (logLevel)
    {
        case LL_Info:  // writes on stdout
            SetConsoleTextAttribute(instance->m_stdoutHandle_Windows, EndOfFormatting_Windows);
            std::cout << CerberusUtils::strPrint("%s [", timestamp.c_str());
            SetConsoleTextAttribute(instance->m_stdoutHandle_Windows,
                                    instance->m_infoLogTerminalFormatting_Windows);
            std::cout << "INFO";
            SetConsoleTextAttribute(instance->m_stdoutHandle_Windows, EndOfFormatting_Windows);
            std::cout << CerberusUtils::strPrint("] %s%s", logAuthor.c_str(), s.c_str());
            std::cout << std::endl;
            break;

        case LL_Warning:  // writes on stdout
            SetConsoleTextAttribute(instance->m_stdoutHandle_Windows, EndOfFormatting_Windows);
            std::cout << CerberusUtils::strPrint("%s [", timestamp.c_str());
            SetConsoleTextAttribute(instance->m_stdoutHandle_Windows,
                                    instance->m_warningLogTerminalFormatting_Windows);
            std::cout << "WARNING";
            SetConsoleTextAttribute(instance->m_stdoutHandle_Windows, EndOfFormatting_Windows);
            std::cout << CerberusUtils::strPrint("] %s%s", logAuthor.c_str(), s.c_str());
            std::cout << std::endl;
            break;

        case LL_Error:  // writes on stderr
            SetConsoleTextAttribute(instance->m_stdoutHandle_Windows, EndOfFormatting_Windows);
            std::cerr << CerberusUtils::strPrint("%s [", timestamp.c_str());
            SetConsoleTextAttribute(instance->m_stdoutHandle_Windows,
                                    instance->m_errorLogTerminalFormatting_Windows);
            std::cerr << "ERROR";
            SetConsoleTextAttribute(instance->m_stdoutHandle_Windows, EndOfFormatting_Windows);
            std::cerr << CerberusUtils::strPrint("] %s%s", logAuthor.c_str(), s.c_str());
            std::cerr << std::endl;
            break;

        case LL_Debug:  // writes on stderr
            SetConsoleTextAttribute(instance->m_stdoutHandle_Windows, EndOfFormatting_Windows);
            std::cerr << CerberusUtils::strPrint("%s [", timestamp.c_str());
            SetConsoleTextAttribute(instance->m_stdoutHandle_Windows,
                                    instance->m_debugLogTerminalFormatting_Windows);
            std::cerr << "DEBUG";
            SetConsoleTextAttribute(instance->m_stdoutHandle_Windows, EndOfFormatting_Windows);
            std::cerr << CerberusUtils::strPrint("] %s%s", logAuthor.c_str(), s.c_str());
            std::cerr << std::endl;
            break;

        default:
            break;
    }
#else

    switch (ll)
    {
        case LL_Debug:
            return CerberusUtils::strPrint("%s%s [%sDEBUG%s] %s%s" NEWLINE, m_endForm, t.c_str(),
                                           m_debForm.c_str(), m_endForm, a.c_str(), str.c_str());

        case LL_Info:
            return CerberusUtils::strPrint("%s%s [%sINFO%s] %s%s" NEWLINE, m_endForm, t.c_str(),
                                           m_infoForm.c_str(), m_endForm, a.c_str(), str.c_str());

        case LL_Error:
            return CerberusUtils::strPrint("%s%s [%sERROR%s] %s%s" NEWLINE, m_endForm, t.c_str(),
                                           m_errForm.c_str(), m_endForm, a.c_str(), str.c_str());

        case LL_Warning:
            return CerberusUtils::strPrint("%s%s [%sWARNING%s] %s%s" NEWLINE, m_endForm, t.c_str(),
                                           m_warnForm.c_str(), m_endForm, a.c_str(), str.c_str());

        default:
            break;
    }

#endif

    return "";
}
//=============================================================================
bool CerberusLog::rawLogNeeded() { return (!m_logConf.colorFormatting || fileLoggerAvail()); }
//=============================================================================
bool CerberusLog::fileLoggerAvail()
{
    if (m_loggerFlag.load())
        if (!m_logger->isFailed()) return true;

    return false;
}
//=============================================================================
FILE* CerberusLog::stream(LogLevel ll)
{
    if (ll == LL_Debug || ll == LL_Error) return stderr;
    return stdout;
}
//=============================================================================
void CerberusLog::sysPrint(FILE* f, const std::string& str) { fprintf(f, "%s", str.c_str()); }
//=============================================================================
std::string CerberusLog::authStr(const std::string& str, bool app)
{
    if (str.empty())
    {
        if (app)
            return "";
        else
            return CERB_TAG_STR " ";
    }
    else
    {
        if (app)
        {
            return std::string("[").append(str).append("] ");
        }
        else
            return std::string(CERB_TAG_STR " [").append(str).append("] ");
    }
}
//=============================================================================
