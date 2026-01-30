#ifndef EXCEPTIONCATALOG_H
#define EXCEPTIONCATALOG_H

#define cIllegalArgExc(text, ...) \
    ::crb::Exception(crb::Exception::ET_IllegalArgument, __LINE__, __FILE__, text, ##__VA_ARGS__)

#define cIllegalStateExc(text, ...) \
    ::crb::Exception(crb::Exception::ET_IllegalState, __LINE__, __FILE__, text, ##__VA_ARGS__)

#define cSystemExc(text, ...) \
    ::crb::Exception(crb::Exception::ET_System, __LINE__, __FILE__, text, ##__VA_ARGS__)

#define cImplMissExc(text, ...)                                                                    \
    ::crb::Exception(crb::Exception::ET_MissingImplementation, __LINE__, __FILE__, text, \
                          ##__VA_ARGS__)

#define cInvalidCastExc(text, ...) \
    ::crb::Exception(crb::Exception::ET_InvalidCast, __LINE__, __FILE__, text, ##__VA_ARGS__)

#define cUsageErrorExc(text, ...) \
    ::crb::Exception(crb::Exception::ET_UsageError, __LINE__, __FILE__, text, ##__VA_ARGS__)

#define cOpResExc(text, ...) \
    ::crb::Exception(crb::Exception::ET_OperationResult, 0, nullptr, text, ##__VA_ARGS__)

#define cFatalExc(text, ...) \
    ::crb::Exception(crb::Exception::ET_Fatal, __LINE__, __FILE__, text, ##__VA_ARGS__)

#endif  // EXCEPTIONCATALOG_H
