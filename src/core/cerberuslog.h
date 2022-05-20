#ifndef CERBERUS_CORE_CERBERUSLOG_H
#define CERBERUS_CORE_CERBERUSLOG_H

#include <string>

namespace cerberus
{
    namespace core
    {
        class CerberusLog
        {
            public:
                CerberusLog();
                CerberusLog(const CerberusLog& other) = delete;

                enum LogLevel
                {
                    LL_Info,
                    LL_Warning,
                    LL_Error,
                    LL_Debug,
                };

                //Logs the given string to stdout/stderr according to the specified logLevel
                static void log(const std::string& str, LogLevel logLevel = LL_Info, const std::string& author = std::string());
        };
    }
}

#endif // CERBERUS_CORE_CERBERUSLOG_H
