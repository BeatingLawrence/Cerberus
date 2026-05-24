#ifndef LOG_H
#define LOG_H

#include "core/cerberusutils.h"

// low-level debug, with no formatting or file log
#define lldebug(text, ...) \
    ::crb::core::CerberusLog::llDebug(::crb::CerberusUtils::strPrint(text, ##__VA_ARGS__))

#if defined(CERBERUS_LIBRARY)
// compiling the framework

#define logInfo(text, ...)                                                                                  \
    ::crb::Cerberus::log(::crb::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::crb::LogLevel::LL_Info, "", \
                         false)
#define logWarning(text, ...)                                                                              \
    ::crb::Cerberus::log(::crb::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::crb::LogLevel::LL_Warning, \
                         "", false)
#define logError(text, ...)                                                                                  \
    ::crb::Cerberus::log(::crb::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::crb::LogLevel::LL_Error, "", \
                         false)
#define logDebug(text, ...)                                                                                  \
    ::crb::Cerberus::log(::crb::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::crb::LogLevel::LL_Debug, "", \
                         false)

#define tlogInfo(text, ...)                                                                                \
    ::crb::Cerberus::log(::crb::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::crb::LogLevel::LL_Info,    \
                         this->getThreadName().empty() ? ::crb::CerberusUtils::strPrint("%lx", this->id()) \
                                                       : this->getThreadName(),                            \
                         false)

#define tlogWarning(text, ...)                                                                             \
    ::crb::Cerberus::log(::crb::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::crb::LogLevel::LL_Warning, \
                         this->getThreadName().empty() ? ::crb::CerberusUtils::strPrint("%lx", this->id()) \
                                                       : this->getThreadName(),                            \
                         false)

#define tlogError(text, ...)                                                                               \
    ::crb::Cerberus::log(::crb::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::crb::LogLevel::LL_Error,   \
                         this->getThreadName().empty() ? ::crb::CerberusUtils::strPrint("%lx", this->id()) \
                                                       : this->getThreadName(),                            \
                         false)

#define tlogDebug(text, ...)                                                                               \
    ::crb::Cerberus::log(::crb::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::crb::LogLevel::LL_Debug,   \
                         this->getThreadName().empty() ? ::crb::CerberusUtils::strPrint("%lx", this->id()) \
                                                       : this->getThreadName(),                            \
                         false)

#else
// compiling the client application

#define logInfo(text, ...)                                                                                  \
    ::crb::Cerberus::log(::crb::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::crb::LogLevel::LL_Info, "", \
                         true)

#define logWarning(text, ...)                                                                              \
    ::crb::Cerberus::log(::crb::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::crb::LogLevel::LL_Warning, \
                         "", true)

#define logError(text, ...)                                                                                  \
    ::crb::Cerberus::log(::crb::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::crb::LogLevel::LL_Error, "", \
                         true)

#define logDebug(text, ...)                                                                                  \
    ::crb::Cerberus::log(::crb::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::crb::LogLevel::LL_Debug, "", \
                         true)

// these log macros must be used inside threads only

#define tlogInfo(text, ...)                                                                                \
    ::crb::Cerberus::log(::crb::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::crb::LogLevel::LL_Info,    \
                         this->getThreadName().empty() ? ::crb::CerberusUtils::strPrint("%lx", this->id()) \
                                                       : this->getThreadName(),                            \
                         true)

#define tlogWarning(text, ...)                                                                             \
    ::crb::Cerberus::log(::crb::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::crb::LogLevel::LL_Warning, \
                         this->getThreadName().empty() ? ::crb::CerberusUtils::strPrint("%lx", this->id()) \
                                                       : this->getThreadName(),                            \
                         true)

#define tlogError(text, ...)                                                                               \
    ::crb::Cerberus::log(::crb::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::crb::LogLevel::LL_Error,   \
                         this->getThreadName().empty() ? ::crb::CerberusUtils::strPrint("%lx", this->id()) \
                                                       : this->getThreadName(),                            \
                         true)

#define tlogDebug(text, ...)                                                                               \
    ::crb::Cerberus::log(::crb::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::crb::LogLevel::LL_Debug,   \
                         this->getThreadName().empty() ? ::crb::CerberusUtils::strPrint("%lx", this->id()) \
                                                       : this->getThreadName(),                            \
                         true)

#endif

#endif  // LOG_H
