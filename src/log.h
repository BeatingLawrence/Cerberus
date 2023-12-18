#ifndef LOG_H
#define LOG_H

#include "core/cerberusutils.h"

// low-level debug, with no cerberus management or file log
#define lldebug(text, ...) ::cerberus::core::CerberusLog::llDebug(::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__))

#if defined(CERBERUS_LIBRARY)
// compiling the framework
#define logInfo(text, ...) ::cerberus::Cerberus::log(::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::cerberus::LogLevel::LL_Info, "CERB", false)
#define logWarning(text, ...) ::cerberus::Cerberus::log(::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::cerberus::LogLevel::LL_Warning, "CERB", false)
#define logError(text, ...) ::cerberus::Cerberus::log(::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::cerberus::LogLevel::LL_Error, "CERB", false)
#define logDebug(text, ...) ::cerberus::Cerberus::log(::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::cerberus::LogLevel::LL_Debug, "CERB", false)

#else
// compiling the client application
#define logInfo(text, ...) ::cerberus::Cerberus::log(::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::cerberus::LogLevel::LL_Info, "", true)
#define logWarning(text, ...) ::cerberus::Cerberus::log(::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::cerberus::LogLevel::LL_Warning, "", true)
#define logError(text, ...) ::cerberus::Cerberus::log(::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::cerberus::LogLevel::LL_Error, "", true)
#define logDebug(text, ...) ::cerberus::Cerberus::log(::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__), ::cerberus::LogLevel::LL_Debug, "", true)

// these log macros must be used inside threads only
#define thrlogInfo(text) ::cerberus::Cerberus::log(text, ::cerberus::LogLevel::LL_Info, this->name(), true)
#define thrlogWarning(text) ::cerberus::Cerberus::log(text, ::cerberus::LogLevel::LL_Warning, this->name(), true)
#define thrlogError(text) ::cerberus::Cerberus::log(text, ::cerberus::LogLevel::LL_Error, this->name(), true)
#define thrlogDebug(text) ::cerberus::Cerberus::log(text, ::cerberus::LogLevel::LL_cdebug, this->name(), true)

#endif

#endif  // LOG_H
