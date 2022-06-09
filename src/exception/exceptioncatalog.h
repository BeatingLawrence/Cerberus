#ifndef EXCEPTIONCATALOG_H
#define EXCEPTIONCATALOG_H

#include "./exception.h"
#include "../core/cerberusutils.h"

//#define cerberusIllegalArgumentExc(text) cerberus::exception::Exception((text), __LINE__, __FILE__, cerberus::exception::Exception::ET_IllegalArgument)
//#define cerberusIllegalStateExc(text) cerberus::exception::Exception((text), __LINE__, __FILE__, cerberus::exception::Exception::ET_IllegalState)
//#define cerberusSystemExc(text) cerberus::exception::Exception((text), __LINE__, __FILE__, cerberus::exception::Exception::ET_System)
//#define cerberusImplementationMissExc(text) cerberus::exception::Exception((text), __LINE__, __FILE__, cerberus::exception::Exception::ET_MissingImplementation)

#define cerberusIllegalArgumentExc(text, ...) ::cerberus::exception::Exception(::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__).c_str(), __LINE__, __FILE__, cerberus::exception::Exception::ET_IllegalArgument)
#define cerberusIllegalStateExc(text, ...) ::cerberus::exception::Exception(::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__).c_str(), __LINE__, __FILE__, cerberus::exception::Exception::ET_IllegalState)
#define cerberusSystemExc(text, ...) ::cerberus::exception::Exception(::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__).c_str(), __LINE__, __FILE__, cerberus::exception::Exception::ET_System)
#define cerberusImplementationMissExc(text, ...) ::cerberus::exception::Exception(::cerberus::core::CerberusUtils::strPrint(text, ##__VA_ARGS__).c_str(), __LINE__, __FILE__, cerberus::exception::Exception::ET_MissingImplementation)

#endif // EXCEPTIONCATALOG_H
