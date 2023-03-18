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

    enum IniDataType : uint8_t
    {
        IDT_NotAType = 0,    //specified when a value has an unknown type
        IDT_String   = 1,    //specified when a value is considered a string
        IDT_Integer  = 2,    //false if key value contains a letter or a symbol
        IDT_Double   = 3,    //false if key value does not contain a '.' or if it contains a letter
        IDT_Bool     = 4,    //true only if key value equals "true" or "false" (case insensitive)
    };

    enum SocketType : uint8_t
    {
        Socket_UDP,
        Socket_TCP,
        Socket_HTTP,
        Socket_HTTPS,
        Socket_WEB,
        Socket_FTP,
        Socket_ICMP,
        Socket_IPC,
    };

    enum SocketOperation : uint8_t
    {
        SO_OK,
        SO_Failure,
        SO_FailedSocket,
        SO_BindFailure,
        SO_ConnectFailure,
        SO_ResolveServerTempFailure,
        SO_ResolveServerFailure,
        SO_ResolveNoData,
        SO_ResolveNotFound,
        SO_ResolveSystemFailure,
        SO_ResolveFailure,
    };

    struct Host
    {
        union
        {
            uint8_t octect[4];  //e.g. address 192.168.4.5 has octect[0]=192 and octect[3]=5
            uint32_t octet_networkOrder;
        };

        uint16_t port;
        std::string hostname;

        //This method takes an ip address in the form of x.x.x.x or x.x.x.x:yyyyy
        //and converts the string filling port and octet[] members
        //It returns true if the conversion performed successfully
        bool fromString(const std::string& str)
        {
            auto col = str.find_last_of(':');

            if(col != std::string::npos && col != str.size() - 1)
            {
                //address with port
                std::string portstr = str.substr(col + 1);
                int portint = atoi(portstr.c_str());

                if(portint < 0 || portint > 65535)
                {
                    return false;
                }

                port = portint;
            }

            int32_t oct[4];
            std::string::size_type dots[3];
            std::string ip = str.substr(0, col);
            std::string sub;
            //
            dots[0] = ip.find_first_of('.');
            sub = ip.substr(0, dots[0]);
            oct[0] = atoi(sub.c_str());

            if(oct[0] < 0 || oct[0] > 255 || sub.empty())
            {
                return false;
            }

            dots[1] = ip.find_first_of('.', dots[0] + 1);
            sub = ip.substr(dots[0] + 1, dots[1] - (dots[0] + 1));
            oct[1] = atoi(sub.c_str());

            if(oct[1] < 0 || oct[1] > 255 || sub.empty())
            {
                return false;
            }

            dots[2] = ip.find_first_of('.', dots[1] + 1);
            sub = ip.substr(dots[1] + 1, dots[2] - (dots[1] + 1));
            oct[2] = atoi(sub.c_str());

            if(oct[2] < 0 || oct[2] > 255 || sub.empty())
            {
                return false;
            }

            sub = ip.substr(dots[2] + 1);
            oct[3] = atoi(sub.c_str());

            if(oct[3] < 0 || oct[3] > 255 || sub.empty())
            {
                return false;
            }

            octect[0] = oct[0];
            octect[1] = oct[1];
            octect[2] = oct[2];
            octect[3] = oct[3];
            return true;
        }
    };
}


#endif // TYPES_H
