#include "./types.h"

#include <netdb.h>

#include <cstring>

#include "./core/cerberusutils.h"
#include "src/core/cerberuslog.h"

#ifdef WINDOWS_SYSTEM
// define constants here
#error "Please define constants"
#else
#include <netinet/in.h>
const uint32_t cerberus::Host::ADDR_ANY       = INADDR_ANY;
const uint32_t cerberus::Host::ADDR_LOOPBACK  = INADDR_LOOPBACK;
const uint32_t cerberus::Host::ADDR_BROADCAST = INADDR_BROADCAST;
#endif

//=============================================================================
cerberus::Host::Host()
    : octet_networkOrder(ADDR_ANY),
      port(0),
      resolved(false)
{
}
//=============================================================================
cerberus::Host::Host(const std::string &str)
    : octet_networkOrder(ADDR_ANY),
      port(0),
      resolved(false)
{
    if (!fromString(str)) clogError("Host %s is invalid", str.c_str());
}
//=============================================================================
cerberus::Host::Host(const char *str)
    : octet_networkOrder(ADDR_ANY),
      port(0),
      resolved(false)
{
    if (!fromString(str)) clogError("Host %s is invalid", str);
}
//=============================================================================
cerberus::Host cerberus::Host::stringToHost(const std::string &str)
{
    Host ret{};
    ret.port = Host::getPort(str);

    std::string ip = str.substr(0, str.find_last_of(':'));
    // search for any, local or broadcast

    if (core::CerberusUtils::areEqual(core::CerberusUtils::toLower(ip), "any"))
    {
        ret.octet_networkOrder = ADDR_ANY;
        return ret;
    }

    if (core::CerberusUtils::areEqual(core::CerberusUtils::toLower(ip), "local"))
    {
        ret.octet_networkOrder = ADDR_LOOPBACK;
        return ret;
    }

    if (core::CerberusUtils::areEqual(core::CerberusUtils::toLower(ip), "broadcast"))
    {
        ret.octet_networkOrder = ADDR_BROADCAST;
        return ret;
    }

    if (core::CerberusUtils::isAlpha(ip))  // Domain name
    {
        ret.hostname = ip;
        return ret;
    }

    int32_t oct[4];
    std::string::size_type dots[3];
    std::string sub;

    //
    dots[0] = ip.find_first_of('.');
    sub     = ip.substr(0, dots[0]);
    oct[0]  = atoi(sub.c_str());

    if (oct[0] < 0 || oct[0] > 255 || sub.empty())
    {
        return Host();
    }

    dots[1] = ip.find_first_of('.', dots[0] + 1);
    sub     = ip.substr(dots[0] + 1, dots[1] - (dots[0] + 1));
    oct[1]  = atoi(sub.c_str());

    if (oct[1] < 0 || oct[1] > 255 || sub.empty())
    {
        return Host();
    }

    dots[2] = ip.find_first_of('.', dots[1] + 1);
    sub     = ip.substr(dots[1] + 1, dots[2] - (dots[1] + 1));
    oct[2]  = atoi(sub.c_str());

    if (oct[2] < 0 || oct[2] > 255 || sub.empty())
    {
        return Host();
    }

    sub    = ip.substr(dots[2] + 1);
    oct[3] = atoi(sub.c_str());

    if (oct[3] < 0 || oct[3] > 255 || sub.empty())
    {
        return Host();
    }

    ret.octect[0] = oct[0];
    ret.octect[1] = oct[1];
    ret.octect[2] = oct[2];
    ret.octect[3] = oct[3];
    return ret;
}
//=============================================================================
uint16_t cerberus::Host::getPort(const std::string &str)
{
    auto col    = str.find_last_of(':');
    int portint = 0;

    if (col != std::string::npos && col != str.size() - 1)
    {
        std::string portstr = str.substr(col + 1);
        portint             = atoi(portstr.c_str());

        if (portint < 0 || portint > 65535)
        {
            return 0;
        }
    }

    return portint;
}
//=============================================================================
bool cerberus::Host::fromString(const std::string &str)
{
    auto h = Host::stringToHost(str);

    if (h.isValid())
    {
        if (h.port) port = h.port;
        octet_networkOrder = h.octet_networkOrder;
        hostname           = h.hostname;
        return true;
    }

    return false;
}
//=============================================================================
std::string cerberus::Host::toString() { return cerberus::core::CerberusUtils::strPrint("%u.%u.%u.%u:%u", octect[0], octect[1], octect[2], octect[3], port); }
//=============================================================================
bool cerberus::Host::isValid() { return (isNumeric() || isTextual() || hasPort()); }
//=============================================================================
bool cerberus::Host::isValidRemote() { return (isNumeric() || isTextual()) && hasPort(); }
//=============================================================================
bool cerberus::Host::isNumeric() { return octet_networkOrder != 0; }
//=============================================================================
bool cerberus::Host::isTextual() { return !hostname.empty(); }
//=============================================================================
bool cerberus::Host::hasPort() { return port != 0; }
//=============================================================================
cerberus::OperationResult cerberus::Host::resolve()
{
    addrinfo *res = nullptr;
    addrinfo info;
    info.ai_family    = AF_INET;
    info.ai_socktype  = 0;
    info.ai_protocol  = 0;
    info.ai_flags     = 0;
    info.ai_addr      = nullptr;
    info.ai_addrlen   = 0;
    info.ai_canonname = nullptr;
    info.ai_next      = nullptr;
    int ret           = getaddrinfo(hostname.c_str(), nullptr, &info, &res);

    if (ret == 0)
    {
        sockaddr_in *addr  = (sockaddr_in *)(res->ai_addr);
        octet_networkOrder = addr->sin_addr.s_addr;
        resolved           = true;
        freeaddrinfo(res);
        return OR_OK;
    }

    if (ret == EAI_AGAIN)
    {
        cdebug("DNS lookup: temporary server failure [%s]", hostname.c_str());
        return OR_ResolveServerTempFailure;
    }
    else if (ret == EAI_FAIL)
    {
        cdebug("DNS lookup: server failure [%s]", hostname.c_str());
        return OR_ResolveServerFailure;
    }
    else if (ret == EAI_NODATA)
    {
        cdebug("DNS lookup: hostname exists but has no ip associated [%s]", hostname.c_str());
        return OR_ResolveNoData;
    }
    else if (ret == EAI_NONAME)
    {
        cdebug("DNS lookup: hostname was not found [%s]", hostname.c_str());
        return OR_ResolveNotFound;
    }
    else if (ret == EAI_SYSTEM)
    {
        cdebug("DNS lookup: system failure, %s [%s]", strerror(errno), hostname.c_str());
        return OR_ResolveSystemFailure;
    }
    else
    {
        cdebug("DNS lookup: failure, %s [%s]", gai_strerror(ret), hostname.c_str());
        return OR_ResolveFailure;
    }
}
//=============================================================================
cerberus::OperationResult::OperationResult()
    : res(OR_Undefined),
      i(0)
{
}
//=============================================================================
cerberus::OperationResult::OperationResult(Result r)
    : res(r),
      i(0)
{
}
//=============================================================================
cerberus::OperationResult::OperationResult(int64_t i)
    : res(OR_OK),
      i(i)
{
}
//=============================================================================
cerberus::OperationResult::OperationResult(double f)
    : res(OR_OK),
      f(f)
{
}
//=============================================================================
cerberus::OperationResult::OperationResult(const std::string &str)
    : res(OR_OK),
      str(str)
{
}
//=============================================================================
bool cerberus::OperationResult::operator==(const OperationResult &other) { return (res == other.res); }
//=============================================================================
bool cerberus::OperationResult::operator!=(const OperationResult &other) { return (res != other.res); }
//=============================================================================
bool cerberus::OperationResult::ok(bool printError)
{
    if (res == Result::OR_OK)
    {
        return true;
    }
    else
    {
        if (printError)
        {
            clogError("Operation failed: %s", errorString().c_str());
        }
    }

    return false;
}
//=============================================================================
bool cerberus::OperationResult::fail(bool printError)
{
    if (!ok())
    {
        if (printError)
        {
            clogError("Operation failed: %s", errorString().c_str());
        }

        return true;
    }

    return false;
}
//=============================================================================
std::string cerberus::OperationResult::errorString()
{
    switch (res)
    {
        case OR_Undefined:
            return "Undefined";
        case OR_OK:
            return "OK";
        case OR_Failure:
            return "Generic failure";
        case OR_FailedInstance:
            return "Operation was requested on a failed instance";
        case OR_WouldBlock:
            return "Operation would block";
        case OR_TimedOut:
            return "Operation Timeout";
        case OR_Unavailable:
            return "Requested operation is not available on this instance";
        case OR_WrongArgument:
            return "Wrong argument/s";
        case OR_InvalidPath:
            return "Path is invalid";
        case OR_SystemFailure:
            return "A system error occurred";
        case OR_BadConditions:
            return "Operation cannot be executed due to bad conditions";
        case OR_ResolveServerTempFailure:
            return "The name server returned a temporary failure indication";
        case OR_ResolveServerFailure:
            return "he name server returned a permanent failure indication";
        case OR_ResolveNoData:
            return "The specified network host exists, but does not have any network addresses defined";
        case OR_ResolveNotFound:
            return "The node or service is not known";
        case OR_ResolveSystemFailure:
            return "System error";
        case OR_ResolveFailure:
            return "Generic resolve failure";
        case OR_RecvZero:
            return "Zero was received";
        case OR_Hangup:
            return "Hangup";
        case OR_NotFound:
            return "Not found";
        case OR_TemporaryUnavailable:
            return "Temporary unavailable";
        case OR_QueryFailure:
            return "Query failure";
        case OR_DBFailure:
            return "Database failure";
        case OR_TableAlreadyPresent:
            return "Table already present";
        case OR_InvalidFile:
            return "Given file is not valid";
    }

    return "Undefined";
}
