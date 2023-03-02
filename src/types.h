#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <string>

namespace cerberus
{

    enum LogLevel
    {
        LL_Info = 0,
        LL_Warning = 1,
        LL_Error = 2,
        LL_Debug = 3,
    };

    struct CerberusLogRole
    {
        uint8_t textFormatting[3];  //up to 3 formatting specifiers, 0 will be ignored, see define.h
        uint8_t foregroundColor;    //color specifier
        uint8_t backgroundColor;    //color specifier
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
        //add more configuration members here..
    };

    enum SlotType
    {
        ST_UCHAR,       //1 byte
        ST_CHAR,        //1 byte
        ST_USHORT,      //2 byte
        ST_SHORT,       //2 byte
        ST_ULONG,       //4 byte
        ST_LONG,        //4 byte
        ST_ULONGLONG,   //8 byte
        ST_LONGLONG,    //8 byte
        ST_FLOAT,       //4 byte
        ST_DOUBLE,      //8 byte
        ST_BOOL,        //1 byte
        ST_VOIDP,       //pointer
        ST_STDSTRINGP,  //pointer
    };

    enum DataType : uint8_t
    {
        DT_NotAType = 0,    //specified when a value has an unknown type
        DT_String   = 1,    //specified when a value is considered a string
        DT_Integer  = 2,    //false if key value contains a letter or a symbol
        DT_Double   = 3,    //false if key value does not contain a '.' or if it contains a letter
        DT_Bool     = 4,    //true only if key value equals "true" or "false" (case insensitive)
    };

}


#endif // TYPES_H
