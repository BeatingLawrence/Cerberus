#ifndef CERBERUS_CORE_CERBERUSLOG_H
#define CERBERUS_CORE_CERBERUSLOG_H

#include <cstdint>
#include <string>

#include "../log.h"  // IWYU pragma: export
#include "../types.h"
#include "loggerthread.h"

namespace cerberus
{
    class Cerberus;

    namespace core
    {
        class CerberusLog
        {
            friend class cerberus::Cerberus;

            typedef void* HANDLE;

           private:
            CerberusLog(const CerberusLog& other) = delete;

            LogConf m_logConf;
            LoggerThread* m_logger;
            std::atomic_flag m_loggerFlag;

#if defined LINUX_SYSTEM || defined APPLE_SYSTEM
            std::string m_infoForm;
            std::string m_warnForm;
            std::string m_errForm;
            std::string m_debForm;
            static const char* m_endForm;
            std::string parseFormdata(const LogRole& data);
#else
            HANDLE m_stdoutHandle_Windows;                             // used for Windows only
            HANDLE m_stderrHandle_Windows;                             // used for Windows only
            uint8_t m_infoLogTerminalFormatting_Windows;               // used for Windows only
            uint8_t m_warningLogTerminalFormatting_Windows;            // used for Windows only
            uint8_t m_errorLogTerminalFormatting_Windows;              // used for Windows only
            uint8_t m_debugLogTerminalFormatting_Windows;              // used for Windows only
            static const uint8_t EndOfFormatting_Windows;              // used for Windows only
            uint8_t parseFormattingData_Windows(const LogRole& data);  // used for Windows only
#endif
            static bool isMultiLine(const std::string& str);

            static void align(std::string& str, LogLevel logLevel, uint32_t authorLen);

            static std::string toRawLog(const std::string& str, LogLevel ll, const std::string& a,
                                        const std::string& t);

            std::string toFormattedLog(const std::string& str, LogLevel ll, const std::string& a,
                                       const std::string& t);

            bool rawLogNeeded();
            bool fileLoggerAvail();

            static FILE* stream(LogLevel ll);

            static void sysPrint(FILE* f, const std::string& str);

            static std::string auth(const std::string& author);

           public:
            CerberusLog();

            ~CerberusLog();

            // Setup the log features
            void setup(const LogConf& parms);

            void start();

            // Stop and destroy the logger thread
            void stop();

            // Log the given string to stdout/stderr according to the specified logLevel
            void log(const std::string& str, LogLevel logLevel = LL_Info,
                     const std::string& author = std::string(), bool application = true);

            static void llDebug(const std::string& str, const std::string& author = std::string());
        };
    }  // namespace core
}  // namespace cerberus

#endif  // CERBERUS_CORE_CERBERUSLOG_H
