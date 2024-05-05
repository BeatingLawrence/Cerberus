#ifndef EXCEPTIONCATALOG_H
#define EXCEPTIONCATALOG_H

#define cIllegalArgExc(text, ...) \
    ::cerberus::Exception(cerberus::Exception::ET_IllegalArgument, __LINE__, __FILE__, text, ##__VA_ARGS__)

#define cIllegalStateExc(text, ...) \
    ::cerberus::Exception(cerberus::Exception::ET_IllegalState, __LINE__, __FILE__, text, ##__VA_ARGS__)

#define cSystemExc(text, ...) \
    ::cerberus::Exception(cerberus::Exception::ET_System, __LINE__, __FILE__, text, ##__VA_ARGS__)

#define cImplMissExc(text, ...)                                                                    \
    ::cerberus::Exception(cerberus::Exception::ET_MissingImplementation, __LINE__, __FILE__, text, \
                          ##__VA_ARGS__)

#define cInvalidCastExc(text, ...) \
    ::cerberus::Exception(cerberus::Exception::ET_InvalidCast, __LINE__, __FILE__, text, ##__VA_ARGS__)

#define cUsageErrorExc(text, ...) \
    ::cerberus::Exception(cerberus::Exception::ET_UsageError, __LINE__, __FILE__, text, ##__VA_ARGS__)

#define cOpResExc(text, ...) \
    ::cerberus::Exception(cerberus::Exception::ET_OperationResult, 0, nullptr, text, ##__VA_ARGS__)

#define cFatalExc(text, ...) \
    ::cerberus::Exception(cerberus::Exception::ET_Fatal, __LINE__, __FILE__, text, ##__VA_ARGS__)

#endif  // EXCEPTIONCATALOG_H
