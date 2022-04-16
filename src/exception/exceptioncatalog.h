#ifndef EXCEPTIONCATALOG_H
#define EXCEPTIONCATALOG_H

#include "./exception.h"

#define cerberusIllegalArgumentExc(text) cerberus::exception::Exception((text), __LINE__, __FILE__, "Illegal argument")
#define cerberusIllegalStateExc(text) cerberus::exception::Exception((text), __LINE__, __FILE__, "Illegal state")
#define cerberusSystemExc(text) cerberus::exception::Exception((text), __LINE__, __FILE__, "System")
#define cerberusImplementationMissExc(text) cerberus::exception::Exception((text), __LINE__, __FILE__, "Missing implementation")

#endif // EXCEPTIONCATALOG_H
