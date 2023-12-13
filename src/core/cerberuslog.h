#ifndef CERBERUS_CORE_CERBERUSLOG_H
#define CERBERUS_CORE_CERBERUSLOG_H

#include <cstdint>
#include <string>

#include "../Cerberus_global.h"
#include "../mutex/mutex.h"
#include "../types.h"
#include "cerberusutils.h"

// #define logInfo(text) ::cerberus::core::CerberusLog::log(text, ::cerberus::core::CerberusLog::LogLevel::LL_Info)
// #define logWarning(text) ::cerberus::core::CerberusLog::log(text, ::cerberus::core::CerberusLog::LogLevel::LL_Warning)
// #define logError(text) ::cerberus::core::CerberusLog::log(text, ::cerberus::core::CerberusLog::LogLevel::LL_Error)
// #define debug(text) ::cerberus::core::CerberusLog::log(text, ::cerberus::core::CerberusLog::LogLevel::LL_cdebug)

#define clogInfo(text, ...) ::cerberus::core::CerberusLog::log(::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::cerberus::LogLevel::LL_Info, "CERB", false)
#define clogWarning(text, ...) ::cerberus::core::CerberusLog::log(::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::cerberus::LogLevel::LL_Warning, "CERB", false)
#define clogError(text, ...) ::cerberus::core::CerberusLog::log(::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::cerberus::LogLevel::LL_Error, "CERB", false)
#define cdebug(text, ...) ::cerberus::core::CerberusLog::log(::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::cerberus::LogLevel::LL_Debug, "CERB", false)

#define logInfo(text, ...) ::cerberus::core::CerberusLog::log(::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::cerberus::LogLevel::LL_Info, "", true)
#define logWarning(text, ...) ::cerberus::core::CerberusLog::log(::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::cerberus::LogLevel::LL_Warning, "", true)
#define logError(text, ...) ::cerberus::core::CerberusLog::log(::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::cerberus::LogLevel::LL_Error, "", true)
#define logDebug(text, ...) ::cerberus::core::CerberusLog::log(::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::cerberus::LogLevel::LL_Debug, "", true)

#define lldebug(text, ...) ::cerberus::core::CerberusLog::lldebug(::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__))

#define thrlogInfo(text) ::cerberus::core::CerberusLog::log(text, ::cerberus::LogLevel::LL_Info, this->name(), true)
#define thrlogWarning(text) ::cerberus::core::CerberusLog::log(text, ::cerberus::LogLevel::LL_Warning, this->name(), true)
#define thrlogError(text) ::cerberus::core::CerberusLog::log(text, ::cerberus::LogLevel::LL_Error, this->name(), true)
#define thrdebug(text) ::cerberus::core::CerberusLog::log(text, ::cerberus::LogLevel::LL_cdebug, this->name(), true)

namespace cerberus
{
    class Cerberus;

    struct CerberusLogRole;

    struct CerberusLogSetup;

    namespace core
    {
        class CERBERUS_EXPORT CerberusLog
        {
            friend class cerberus::Cerberus;

           public:
            typedef void* HANDLE;

           private:
            CerberusLog(const CerberusLog& other) = delete;

            bool m_useFormattedTerminal;
            bool m_fileLogEnable;
            LogLevel m_appLogLevel;
            LogLevel m_cerbLogLevel;

            std::string m_infoLogTerminalFormatting_Linux;                        // used for Linux only
            std::string m_warningLogTerminalFormatting_Linux;                     // used for Linux only
            std::string m_errorLogTerminalFormatting_Linux;                       // used for Linux only
            std::string m_debugLogTerminalFormatting_Linux;                       // used for Linux only
            static const char* EndOfFormatting_Linux;                             // used for Linux only
            std::string _parseFormattingData_Linux(const CerberusLogRole& data);  // used for Linux only

            HANDLE m_stdoutHandle_Windows;                                      // used for Windows only
            HANDLE m_stderrHandle_Windows;                                      // used for Windows only
            uint8_t m_infoLogTerminalFormatting_Windows;                        // used for Windows only
            uint8_t m_warningLogTerminalFormatting_Windows;                     // used for Windows only
            uint8_t m_errorLogTerminalFormatting_Windows;                       // used for Windows only
            uint8_t m_debugLogTerminalFormatting_Windows;                       // used for Windows only
            static const uint8_t EndOfFormatting_Windows;                       // used for Windows only
            uint8_t _parseFormattingData_Windows(const CerberusLogRole& data);  // used for Windows only

            cerberus::mutex::Mutex m_mutex;

            static CerberusLog* _instance();

            CerberusLog();

            ~CerberusLog();

            // Setups the logging singleton using given structure
            static void _setup(const CerberusLogSetup& setup);

            static bool isMultiLine(const std::string& str);

            static void align(std::string& str, LogLevel logLevel, uint32_t authorLen);

           public:
            // Logs the given string to stdout/stderr according to the specified logLevel
            static void log(const std::string& str, LogLevel logLevel = LL_Info, const std::string& author = std::string(), bool application = true);

            static void llDebug(const std::string& str, const std::string& author = std::string());
        };
    }  // namespace core
}  // namespace cerberus

#endif  // CERBERUS_CORE_CERBERUSLOG_H
