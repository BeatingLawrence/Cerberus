#ifndef EXCEPTIONCATALOG_H
#define EXCEPTIONCATALOG_H

#include "../core/cerberusutils.h"

#define cerberusIllegalArgExc(text, ...)                                                  \
    ::cerberus::exception::Exception(                                                     \
        ::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__).c_str(), __LINE__, \
        __FILE__, cerberus::exception::Exception::ET_IllegalArgument)
#define cerberusIllegalStateExc(text, ...)                                                \
    ::cerberus::exception::Exception(                                                     \
        ::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__).c_str(), __LINE__, \
        __FILE__, cerberus::exception::Exception::ET_IllegalState)
#define cerberusSystemExc(text, ...)                                                      \
    ::cerberus::exception::Exception(                                                     \
        ::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__).c_str(), __LINE__, \
        __FILE__, cerberus::exception::Exception::ET_System)
#define cerberusImplMissExc(text, ...)                                                    \
    ::cerberus::exception::Exception(                                                     \
        ::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__).c_str(), __LINE__, \
        __FILE__, cerberus::exception::Exception::ET_MissingImplementation)
#define cerberusInvalidCastExc(text, ...)                                                 \
    ::cerberus::exception::Exception(                                                     \
        ::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__).c_str(), __LINE__, \
        __FILE__, cerberus::exception::Exception::ET_InvalidCast)
#define cerberusUsageErrorExc(text, ...)                                                  \
    ::cerberus::exception::Exception(                                                     \
        ::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__).c_str(), __LINE__, \
        __FILE__, cerberus::exception::Exception::ET_UsageError)
#define cerberusOpResExc(text, ...)                                                         \
    ::cerberus::exception::Exception(                                                       \
        ::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__).c_str(), 0, nullptr, \
        cerberus::exception::Exception::ET_OperationResult)

#endif  // EXCEPTIONCATALOG_H
