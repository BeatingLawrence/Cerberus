#include "cerberuslog.h"
#include "../mutex/mutex.h"
#include "../mutex/mutexlocker.h"
#include "./cerberusutils.h"
#include <iostream>

using namespace cerberus::core;

const char* Cerberus::EndOfFormatting_Linux = "\033[0m";

//=============================================================================
void CerberusLog::log(const std::string& str, LogLevel logLevel, const std::string& author)
{
    static mutex::Mutex mutex;
    mutex::MutexLocker locker(&mutex);
    //time
    auto now = std::chrono::system_clock::now();
    auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
    auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(now - seconds);
    time_t coarseTime = std::chrono::system_clock::to_time_t(now);
    tm* local = std::localtime(&coarseTime);
    uint32_t fineTime = milli.count();
    std::string timestamp = CerberusUtils::strPrint("%.4u.%.2u.%.2u-%.2u:%.2u:%.2u.%.3u",
                            local->tm_year + 1900,
                            local->tm_mon + 1,
                            local->tm_mday,
                            local->tm_hour,
                            local->tm_min,
                            local->tm_sec,
                            fineTime);
    std::string logAuthor;

    if(!author.empty())
    {
        logAuthor = '[';
        logAuthor += author;
        logAuthor += "] ";
    }

    //Log on file
    std::string rawLog;

    switch(logLevel)
    {
        case LL_Info:
            rawLog =  CerberusUtils::strPrint("%s [INFO] %s%s", timestamp.c_str(), logAuthor.c_str(), str.c_str());
            break;

        case LL_Warning:
            rawLog =  CerberusUtils::strPrint("%s [WARNING] %s%s", timestamp.c_str(), logAuthor.c_str(), str.c_str());
            break;

        case LL_Error:
            rawLog =  CerberusUtils::strPrint("%s [ERROR] %s%s", timestamp.c_str(), logAuthor.c_str(), str.c_str());
            break;

        case LL_Debug:
            rawLog =  CerberusUtils::strPrint("%s [DEBUG] %s%s", timestamp.c_str(), logAuthor.c_str(), str.c_str());
            break;
    }

    message::cerberus_message logMessage = message::StandardMessageFactory::createStandardMessage(CERBERUS_MESSAGE_LOG_ID);
    logMessage->getSlotAt(0)->to<message::slot::StringSlot>()->setValue(rawLog);
    Cerberus::send(logMessage);

    if(!(cerberus->m_useFormattedTerminal))
    {
        std::cout << rawLog.c_str() << std::endl;
        return;
    }

#ifndef WINDOWS_SYSTEM

    switch(logLevel)
    {
        case LL_Info:       //writes on stdout
            std::cout << strPrint("%s%s [%sINFO%s] %s%s",
                                  EndOfFormatting_Linux,
                                  timestamp.c_str(),
                                  cerberus->m_infoLogTerminalFormatting_Linux.c_str(),
                                  EndOfFormatting_Linux,
                                  logAuthor.c_str(),
                                  str.c_str()) << std::endl;
            break;

        case LL_Warning:    //writes on stdout
            std::cout << strPrint("%s%s [%sWARNING%s] %s%s",
                                  EndOfFormatting_Linux,
                                  timestamp.c_str(),
                                  cerberus->m_warningLogTerminalFormatting_Linux.c_str(),
                                  EndOfFormatting_Linux,
                                  logAuthor.c_str(),
                                  str.c_str()) << std::endl;
            break;

        case LL_Error:      //writes on stderr
            std::cerr << strPrint("%s%s [%sERROR%s] %s%s",
                                  EndOfFormatting_Linux,
                                  timestamp.c_str(),
                                  cerberus->m_errorLogTerminalFormatting_Linux.c_str(),
                                  EndOfFormatting_Linux,
                                  logAuthor.c_str(),
                                  str.c_str()) << std::endl;
            break;

        case LL_Debug:      //writes on stdout
            std::cout << strPrint("%s%s [%sDEBUG%s] %s%s",
                                  EndOfFormatting_Linux,
                                  timestamp.c_str(),
                                  cerberus->m_debugLogTerminalFormatting_Linux.c_str(),
                                  EndOfFormatting_Linux,
                                  logAuthor.c_str(),
                                  str.c_str()) << std::endl;
            break;
    }

#else

    switch(logLevel)
    {
        case LL_Info:       //writes on stdout
            SetConsoleTextAttribute(cerberus->m_stdoutHandle_Windows, EndOfFormatting_Windows);
            std::cout << strPrint("%s [", timestamp.c_str());
            SetConsoleTextAttribute(cerberus->m_stdoutHandle_Windows, cerberus->m_infoLogTerminalFormatting_Windows);
            std::cout << "INFO";
            SetConsoleTextAttribute(cerberus->m_stdoutHandle_Windows, EndOfFormatting_Windows);
            std::cout << strPrint("] %s%s", logAuthor.c_str(), str.c_str());
            std::cout << std::endl;
            break;

        case LL_Warning:    //writes on stdout
            SetConsoleTextAttribute(cerberus->m_stdoutHandle_Windows, EndOfFormatting_Windows);
            std::cout << strPrint("%s [", timestamp.c_str());
            SetConsoleTextAttribute(cerberus->m_stdoutHandle_Windows, cerberus->m_warningLogTerminalFormatting_Windows);
            std::cout << "WARNING";
            SetConsoleTextAttribute(cerberus->m_stdoutHandle_Windows, EndOfFormatting_Windows);
            std::cout << strPrint("] %s%s", logAuthor.c_str(), str.c_str());
            std::cout << std::endl;
            break;

        case LL_Error:      //writes on stderr
            SetConsoleTextAttribute(cerberus->m_stdoutHandle_Windows, EndOfFormatting_Windows);
            std::cerr << strPrint("%s [", timestamp.c_str());
            SetConsoleTextAttribute(cerberus->m_stdoutHandle_Windows, cerberus->m_errorLogTerminalFormatting_Windows);
            std::cerr << "ERROR";
            SetConsoleTextAttribute(cerberus->m_stdoutHandle_Windows, EndOfFormatting_Windows);
            std::cerr << strPrint("] %s%s", logAuthor.c_str(), str.c_str());
            std::cerr << std::endl;
            break;

        case LL_Debug:      //writes on stdout
            SetConsoleTextAttribute(cerberus->m_stdoutHandle_Windows, EndOfFormatting_Windows);
            std::cout << strPrint("%s [", timestamp.c_str());
            SetConsoleTextAttribute(cerberus->m_stdoutHandle_Windows, cerberus->m_debugLogTerminalFormatting_Windows);
            std::cout << "DEBUG";
            SetConsoleTextAttribute(cerberus->m_stdoutHandle_Windows, EndOfFormatting_Windows);
            std::cout << strPrint("] %s%s", logAuthor.c_str(), str.c_str());
            std::cout << std::endl;
            break;
    }

#endif
}
