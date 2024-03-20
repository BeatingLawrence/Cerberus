#ifndef EXCEPTIONCATALOG_H
#define EXCEPTIONCATALOG_H

#define cerberusIllegalArgExc(text, ...) \
    ::cerberus::Exception(cerberus::Exception::ET_IllegalArgument, __LINE__, __FILE__, text, ##__VA_ARGS__)

#define cerberusIllegalStateExc(text, ...) \
    ::cerberus::Exception(cerberus::Exception::ET_IllegalState, __LINE__, __FILE__, text, ##__VA_ARGS__)

#define cerberusSystemExc(text, ...) \
    ::cerberus::Exception(cerberus::Exception::ET_System, __LINE__, __FILE__, text, ##__VA_ARGS__)

#define cerberusImplMissExc(text, ...)                                                             \
    ::cerberus::Exception(cerberus::Exception::ET_MissingImplementation, __LINE__, __FILE__, text, \
                          ##__VA_ARGS__)

#define cerberusInvalidCastExc(text, ...) \
    ::cerberus::Exception(cerberus::Exception::ET_InvalidCast, __LINE__, __FILE__, text, ##__VA_ARGS__)

#define cerberusUsageErrorExc(text, ...) \
    ::cerberus::Exception(cerberus::Exception::ET_UsageError, __LINE__, __FILE__, text, ##__VA_ARGS__)

#define cerberusOpResExc(text, ...) \
    ::cerberus::Exception(cerberus::Exception::ET_OperationResult, 0, nullptr, text, ##__VA_ARGS__)

#endif  // EXCEPTIONCATALOG_H
