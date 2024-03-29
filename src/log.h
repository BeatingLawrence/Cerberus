#ifndef LOG_H
#define LOG_H

#include "core/cerberusutils.h"

// low-level debug, with no formatting or file log
#define lldebug(text, ...) \
    ::cerberus::core::CerberusLog::llDebug(::cerberus::CerberusUtils::strPrint(text, ##__VA_ARGS__))

#if defined(CERBERUS_LIBRARY)
// compiling the framework

#define logInfo(text, ...)                                                              \
    ::cerberus::Cerberus::log(::cerberus::CerberusUtils::strPrint(text, ##__VA_ARGS__), \
                              ::cerberus::LogLevel::LL_Info, "", false)
#define logWarning(text, ...)                                                           \
    ::cerberus::Cerberus::log(::cerberus::CerberusUtils::strPrint(text, ##__VA_ARGS__), \
                              ::cerberus::LogLevel::LL_Warning, "", false)
#define logError(text, ...)                                                             \
    ::cerberus::Cerberus::log(::cerberus::CerberusUtils::strPrint(text, ##__VA_ARGS__), \
                              ::cerberus::LogLevel::LL_Error, "", false)
#define logDebug(text, ...)                                                             \
    ::cerberus::Cerberus::log(::cerberus::CerberusUtils::strPrint(text, ##__VA_ARGS__), \
                              ::cerberus::LogLevel::LL_Debug, "", false)

#define tlogInfo(text, ...)                                                             \
    ::cerberus::Cerberus::log(::cerberus::CerberusUtils::strPrint(text, ##__VA_ARGS__), \
                              ::cerberus::LogLevel::LL_Info, this->name(), false)

#define tlogWarning(text, ...)                                                          \
    ::cerberus::Cerberus::log(::cerberus::CerberusUtils::strPrint(text, ##__VA_ARGS__), \
                              ::cerberus::LogLevel::LL_Warning, this->name(), false)

#define tlogError(text, ...)                                                            \
    ::cerberus::Cerberus::log(::cerberus::CerberusUtils::strPrint(text, ##__VA_ARGS__), \
                              ::cerberus::LogLevel::LL_Error, this->name(), false)

#define tlogDebug(text, ...)                                                            \
    ::cerberus::Cerberus::log(::cerberus::CerberusUtils::strPrint(text, ##__VA_ARGS__), \
                              ::cerberus::LogLevel::LL_Debug, this->name(), false)

#else
// compiling the client application

#define logInfo(text, ...)                                                              \
    ::cerberus::Cerberus::log(::cerberus::CerberusUtils::strPrint(text, ##__VA_ARGS__), \
                              ::cerberus::LogLevel::LL_Info, "", true)

#define logWarning(text, ...)                                                           \
    ::cerberus::Cerberus::log(::cerberus::CerberusUtils::strPrint(text, ##__VA_ARGS__), \
                              ::cerberus::LogLevel::LL_Warning, "", true)

#define logError(text, ...)                                                             \
    ::cerberus::Cerberus::log(::cerberus::CerberusUtils::strPrint(text, ##__VA_ARGS__), \
                              ::cerberus::LogLevel::LL_Error, "", true)

#define logDebug(text, ...)                                                             \
    ::cerberus::Cerberus::log(::cerberus::CerberusUtils::strPrint(text, ##__VA_ARGS__), \
                              ::cerberus::LogLevel::LL_Debug, "", true)

// these log macros must be used inside threads only

#define tlogInfo(text, ...)                                                             \
    ::cerberus::Cerberus::log(::cerberus::CerberusUtils::strPrint(text, ##__VA_ARGS__), \
                              ::cerberus::LogLevel::LL_Info, this->name(), true)

#define tlogWarning(text, ...)                                                          \
    ::cerberus::Cerberus::log(::cerberus::CerberusUtils::strPrint(text, ##__VA_ARGS__), \
                              ::cerberus::LogLevel::LL_Warning, this->name(), true)

#define tlogError(text, ...)                                                            \
    ::cerberus::Cerberus::log(::cerberus::CerberusUtils::strPrint(text, ##__VA_ARGS__), \
                              ::cerberus::LogLevel::LL_Error, this->name(), true)

#define tlogDebug(text, ...)                                                            \
    ::cerberus::Cerberus::log(::cerberus::CerberusUtils::strPrint(text, ##__VA_ARGS__), \
                              ::cerberus::LogLevel::LL_Debug, this->name(), true)

#endif

#endif  // LOG_H
