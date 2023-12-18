#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <string>
#include <vector>

#include "mutex/mutexlocker.h"

namespace cerberus
{
    typedef uint32_t SIZE;
    typedef uint64_t LSIZE;
    typedef uint8_t BYTE;

    typedef void (*timerCallback)();

    struct DictLine
    {
        std::string key, val;
    };

    struct DoubleString
    {
        std::string left;
        std::string right;
    };

    typedef std::vector<DictLine> Dictionary;

    struct LoaderFunc
    {
        void* func;
        mutex::MutexLocker mutexLocker;

        bool isValid() { return func != nullptr; };
    };

    enum WordMatch
    {
        WM_CaseSensitive,
        WM_CaseInsensitive,
    };

    enum FileOpenMode
    {
        FOM_Read = 0,         // Open the file for reading only; the file must exist
        FOM_ReadWrite,        // Open the file for reading and writing; the file must exist
        FOM_ReadWriteTrunc,   // Open the file for reading and writing; if the file exists the content is discarded, otherwise, the file is created
        FOM_ReadWriteAppend,  // Open the file for reading and writing; if the file does not esist, it is created.
                              // All the write operations happen at the end of the file
    };

    enum StandardMessage
    {
        SM_LogMsg,
        SM_TerminationMsg,
        // add more custom messages here
    };

    enum Radix
    {
        Decimal,
        Hexadecimal,
        Binary,
    };

    enum LogLevel
    {
        LL_Info    = 0,
        LL_Warning = 1,
        LL_Error   = 2,
        LL_Debug   = 3,
    };

    struct CerberusLogRole
    {
        uint8_t textFormatting[3];  // up to 3 formatting specifiers, 0 will be ignored, see define.h
        uint8_t foregroundColor;    // color specifier
        uint8_t backgroundColor;    // color specifier
    };

    struct CerberusLogSetup
    {
        LogLevel applicationLogLevel;  // set the application log level. Minor levels will be silenced
        LogLevel cerberusLogLevel;     // set the Cerberus framework log level. Minor levels will be silenced
        bool colorFormatting;          // enable the color formatting of the output terminal
        bool logOnFile;                // enable the log on file
        std::string logFileName;       // set the log file name
        SIZE logFileMaximumSize;       // set to zero to disable (not recommended)
        CerberusLogRole infoRole;      // set the log role for the info level
        CerberusLogRole warningRole;   // set the log role for the warning level
        CerberusLogRole errorRole;     // set the log role for the error level
        CerberusLogRole debugRole;     // set the log role for the debug level
    };

    struct CerberusInitParms
    {
        CerberusLogSetup logSetup;
        bool useCiphers;  // enable cerberus to init and use the OpenSSL library
        // add more configuration members here..
    };

    enum SlotType
    {
        ST_UCHAR,       // 1 byte
        ST_CHAR,        // 1 byte
        ST_USHORT,      // 2 byte
        ST_SHORT,       // 2 byte
        ST_ULONG,       // 4 byte
        ST_LONG,        // 4 byte
        ST_ULONGLONG,   // 8 byte
        ST_LONGLONG,    // 8 byte
        ST_FLOAT,       // 4 byte
        ST_DOUBLE,      // 8 byte
        ST_BOOL,        // 1 byte
        ST_VOIDP,       // pointer
        ST_STDSTRINGP,  // pointer
        ST_BYTEBUFFER,  // ByteBuffer object
    };

    enum IniDataType : uint8_t
    {
        IDT_Invalid = 0,  // specified when a value has an unknown type
        IDT_Integer = 2,  // false if key value contains a letter or a symbol
        IDT_Double  = 3,  // false if key value does not contain a '.' or if it contains a letter
        IDT_Bool    = 4,  // true only if key value equals "true" or "false" (case insensitive)
    };

    // The Result enum contains all the possible results of operation requested to the framework.
    enum Result : uint8_t
    {
        OR_Undefined,                 // [general] this result should never be given (used for unimplemented methods)
        OR_OK,                        // [general] no errors
        OR_Failure,                   // [general] generic failure
        OR_FailedInstance,            // [general] attempt to use a failed instance
        OR_WouldBlock,                // [general] attempt to run a blocking operation on a non-blocking call
        OR_TimedOut,                  // [general] operation timeout
        OR_Unavailable,               // [general] the requested operation is not available for the object
        OR_WrongArgument,             // [general] at least one wrong argument
        OR_InvalidPath,               // [general] the file does not exist or the given path is not valid
        OR_SystemFailure,             // [general] a system error occurred
        OR_BadConditions,             // [general] bad conditions encountered when processing the operation
        OR_NotFound,                  // [general] the item was not found
        OR_TemporaryUnavailable,      // [general] the requested operation is not available at the moment, retry later
        OR_InvalidFile,               // [general] the provided file instance is not valid
        OR_Duplicate,                 // [general] the item is a duplicate
        OR_WrongType,                 // [general] the item type is wrong
                                      //
        OR_EOF,                       // [file] EOF reached
        OR_NotEmpty,                  // [file] the provided directory is not empty
                                      //
        OR_ResolveServerTempFailure,  // [DNS lookup] resolve method error
        OR_ResolveServerFailure,      // [DNS lookup] resolve method error
        OR_ResolveNoData,             // [DNS lookup] resolve method error
        OR_ResolveNotFound,           // [DNS lookup] resolve method error
        OR_ResolveSystemFailure,      // [DNS lookup] resolve method error
        OR_ResolveFailure,            // [DNS lookup] resolve method error
                                      //
        OR_RecvZero,                  // [socket] a recv call returned zero
        OR_Hangup,                    // [socket] hangup condition (stream closed by the peer)
                                      //
        OR_QueryFailure,              // [database] query error
        OR_DBFailure,                 // [database] DB error
        OR_TableAlreadyPresent,       // [database] the table is already present
                                      //
        OR_ThreadNotJoinable,         // [thread] the thread is not joinable
    };

    enum MutexType : uint8_t
    {
        Simple,     // ERRORCHECK mutex
        Recursive,  // RECURSIVE mutex
    };

    // The OperationResult object contains a Result member and some data.
    struct OperationResult
    {
        Result res;

        union  // save space. Access one member only
        {
            int64_t i;
            long double f;
            LSIZE sz;
        };

        std::string str;

        OperationResult();  // construct undefined result

        OperationResult(Result r);  // construct defined result

        OperationResult(int64_t i);

        OperationResult(long double f);

        explicit OperationResult(LSIZE sz);

        OperationResult(const std::string& str);

        bool operator==(const OperationResult& other);

        bool operator!=(const OperationResult& other);

        // Throw a generic exception with the given text only if the result is failed
        OperationResult& expect(const std::string& str);

        // Throw a generic exception with the given text only if the result fails and
        // only if it fails with the given reason
        OperationResult& expect(Result reason, const std::string& str);

        // Throw a generic exception with the result status text only if the result fails
        OperationResult& expect();

        // Return true if the Result is OR_OK, false otherwise.
        // If print is true, the error will be printer with logError
        bool ok(bool print = false);

        // Return false if the Result is OR_OK, true otherwise.
        // If print is true, the error will be printer with logError
        bool fail(bool print = false);

        // Translate the Result
        std::string errorString();

        // Used for bool value
        bool isTrue() { return i != 0; };
    };

    struct Host
    {
        static const uint32_t ADDR_ANY;
        static const uint32_t ADDR_LOOPBACK;
        static const uint32_t ADDR_BROADCAST;

        union
        {
            uint8_t octect[4];  // e.g. address 192.168.4.5 has octect[0]=192 and octect[3]=5
            uint32_t octet_networkOrder;
        };

        uint16_t port;
        std::string hostname;

        bool resolved;

        // Construct an invalid Host (0.0.0.0:0)
        Host();

        // Construct an Host with str as hostname if it contains at least one letter,
        // otherwise str will be used to extract ip:port with a stringToHost() call.
        Host(const std::string& str);

        // Same as above
        Host(const char* str);

        // Take an ip address in the form of x.x.x.x or x.x.x.x:yyyyy
        // and return an Host object with numeric IP and port members filled
        // It is possible to use "any", "local" or "broadcast" (case insensitive) in place of the
        // IP address to specify 'any interface', 'localhost' and '255.255.255.255' respectively.
        // An invalid Host is returned if the conversion fails
        static Host stringToHost(const std::string& str);

        // Extract the port number from the given string.
        // The port number must be the last element in the string and must
        // be preceded by a column :
        // If no port is found, 0 is returned
        static uint16_t getPort(const std::string& str);

        // Extract a numeric IP address and port and saves them in this Host instance.
        // The instance becomes invalid if the conversion fails
        bool fromString(const std::string& str);

        // Print the numeric IP address and port (not the hostname) to string.
        std::string toString();

        // Tells if the Host is not valid for any usage
        bool isValid();

        // Tell if the Host is valid for remote usage, e.g. connect() or sendTo().
        // For this method to return true, the Host must have a valid port
        // and either an hostname OR a numerical IP address
        bool isValidRemote();

        // Tell if the Host has a valid numerical IP
        bool isNumeric();

        // Tell if the Host has an hostname
        bool isTextual();

        // Tell if the Host has a valid port (port != 0)
        bool hasPort();

        // Resolve the given Host using the hostname member.
        // The resulting numeric IP address is written in the ip parameter
        OperationResult resolve();
    };

}  // namespace cerberus
#endif  // TYPES_H
