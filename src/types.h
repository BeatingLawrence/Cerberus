#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <string>

namespace cerberus
{
    typedef uint32_t SIZE;
    typedef uint64_t LSIZE;

    enum FileOpenMode
    {
        FOM_Read = 0,         // Open the file for reading only; the file must exist
        FOM_ReadWrite,        // Open the file for reading and writing; the file must exist
        FOM_ReadWriteTrunc,   // Open the file for reading and writing; if the file exist the content is discarded, otherwise, the file is created
        FOM_ReadWriteAppend,  // Open the file for reading and writing; if the file does not esist, it is created.
                              // All the write operations happen at the end of the file
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
        LogLevel logLevel;
        bool disableFormatting;
        std::string logFileName;
        CerberusLogRole infoRole;
        CerberusLogRole warningRole;
        CerberusLogRole errorRole;
        CerberusLogRole debugRole;
    };

    struct CerberusInitParms
    {
        CerberusLogSetup logSetup;
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
    };

    enum IniDataType : uint8_t
    {
        IDT_NotAType = 0,  // specified when a value has an unknown type
        IDT_String   = 1,  // specified when a value is considered a string
        IDT_Integer  = 2,  // false if key value contains a letter or a symbol
        IDT_Double   = 3,  // false if key value does not contain a '.' or if it contains a letter
        IDT_Bool     = 4,  // true only if key value equals "true" or "false" (case insensitive)
    };

    enum Result
    {
        OR_Undefined,                 // [general] this result should never be given
        OR_OK,                        // [general] no errors
        OR_Failure,                   // [general] generic failure
        OR_FailedInstance,            // [general] attempt to do something on a failed instance
        OR_WouldBlock,                // [general] attempt to run a blocking operation on a non-blocking call
        OR_TimedOut,                  // [general] operation timeout
        OR_Unavailable,               // [general] the requested operation is not available for the object
        OR_WrongArgument,             // [general] at least one argument wrong
                                      //
        OR_ResolveServerTempFailure,  // [socket DNS lookup] resolve method error
        OR_ResolveServerFailure,      // [socket DNS lookup] resolve method error
        OR_ResolveNoData,             // [socket DNS lookup] resolve method error
        OR_ResolveNotFound,           // [socket DNS lookup] resolve method error
        OR_ResolveSystemFailure,      // [socket DNS lookup] resolve method error
        OR_ResolveFailure,            // [socket DNS lookup] resolve method error
                                      //
        OR_RecvZero,                  // [socket] a recv call returned zero
    };

    struct OperationResult
    {
        Result res;

        union  // use one member per call
        {
            bool boolvalue;
            int intvalue;
            float floatvalue;
        };

        OperationResult();  // construct undefined result

        OperationResult(Result r);  // construct defined result

        OperationResult(bool b);

        OperationResult(int i);

        OperationResult(float f);

        bool operator==(const OperationResult& other);

        bool operator!=(const OperationResult& other);
    };

    struct Host
    {
        static const uint32_t ADDR_ANY;
        static const uint32_t ADDR_LOOPBACK;
        static const uint32_t ADDR_BROADCAST;

        // Construct an invalid Host (0.0.0.0:0)
        Host();

        // Construct an Host with str as hostname if it contains at least one letter,
        // otherwise str will be used to extract ip:port as a fromString() call
        Host(const std::string& str);

        // Same as above
        Host(const char* str);

        union
        {
            uint8_t octect[4];  // e.g. address 192.168.4.5 has octect[0]=192 and octect[3]=5
            uint32_t octet_networkOrder;
        };

        uint16_t port;
        std::string hostname;

        bool resolved;

        // This method takes an ip address in the form of x.x.x.x or x.x.x.x:yyyyy
        // and converts the string filling port and octet[] members.
        // The port presence is not mandatory
        // It is possible to use "any", "local" or "broadcast" (case insensitive) in place of
        // ip address to specify 'any interface', 'localhost' and '255.255.255.255' respectively.
        // It returns true if the conversion performed successfully
        bool fromString(const std::string& str);

        // Prints the numeric IP address and port, does not print the hostname
        std::string toString();

        // Tells if the Host is not valid, i.e. 0.0.0.0:0 and an empty hostname
        bool isValid();

        // Resolve the given Host. The resulting numeric IP address is written in the ip parameter
        OperationResult resolve();
    };
}  // namespace cerberus

#endif  // TYPES_H
