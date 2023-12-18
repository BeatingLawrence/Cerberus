#ifndef CERBERUS_CORE_CERBERUSLOG_H
#define CERBERUS_CORE_CERBERUSLOG_H

#include <cstdint>
#include <string>

#include "../types.h"
#include "loggerthread.h"

namespace cerberus
{
    class Cerberus;

    struct CerberusLogRole;

    struct CerberusLogSetup;

    namespace core
    {
        class CerberusLog
        {
            friend class cerberus::Cerberus;

            typedef void* HANDLE;

           private:
            CerberusLog(const CerberusLog& other) = delete;

            CerberusLogSetup m_setupParms;
            LoggerThread* m_logger;

            std::string m_infoLogTerminalFormatting_Linux;                       // used for Linux only
            std::string m_warningLogTerminalFormatting_Linux;                    // used for Linux only
            std::string m_errorLogTerminalFormatting_Linux;                      // used for Linux only
            std::string m_debugLogTerminalFormatting_Linux;                      // used for Linux only
            static const char* EndOfFormatting_Linux;                            // used for Linux only
            std::string parseFormattingData_Linux(const CerberusLogRole& data);  // used for Linux only

            HANDLE m_stdoutHandle_Windows;                                     // used for Windows only
            HANDLE m_stderrHandle_Windows;                                     // used for Windows only
            uint8_t m_infoLogTerminalFormatting_Windows;                       // used for Windows only
            uint8_t m_warningLogTerminalFormatting_Windows;                    // used for Windows only
            uint8_t m_errorLogTerminalFormatting_Windows;                      // used for Windows only
            uint8_t m_debugLogTerminalFormatting_Windows;                      // used for Windows only
            static const uint8_t EndOfFormatting_Windows;                      // used for Windows only
            uint8_t parseFormattingData_Windows(const CerberusLogRole& data);  // used for Windows only

            static bool isMultiLine(const std::string& str);

            static void align(std::string& str, LogLevel logLevel, uint32_t authorLen);

           public:
            CerberusLog();

            ~CerberusLog();

            // Setup the log features
            void setup(const CerberusLogSetup& parms);

            void start();

            // Stop and destroy the logger thread
            void stop();

            // Log the given string to stdout/stderr according to the specified logLevel
            void log(const std::string& str, LogLevel logLevel = LL_Info, const std::string& author = std::string(), bool application = true);

            static void llDebug(const std::string& str, const std::string& author = std::string());
        };
    }  // namespace core
}  // namespace cerberus

#endif  // CERBERUS_CORE_CERBERUSLOG_H
