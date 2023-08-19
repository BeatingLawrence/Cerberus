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
    auto h = stringToHost(str);

    if (!fromString(str))
        if (core::CerberusUtils::isAlpha(str))
        {
            hostname = str;
        }
}
//=============================================================================
cerberus::Host::Host(const char *str)
    : octet_networkOrder(ADDR_ANY),
      port(0),
      resolved(false)
{
    if (!fromString(str))
        if (core::CerberusUtils::isAlpha(str))
        {
            hostname = str;
        }
}
//=============================================================================
cerberus::Host cerberus::Host::stringToHost(const std::string &str)
{
    Host ret{};
    auto col = str.find_last_of(':');
    ret.port = Host::getPort(str);

    int32_t oct[4];
    std::string::size_type dots[3];
    std::string ip = str.substr(0, col);
    std::string sub;
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
        debug("DNS lookup: temporary server failure [%s]", hostname.c_str());
        return OR_ResolveServerTempFailure;
    }
    else if (ret == EAI_FAIL)
    {
        debug("DNS lookup: server failure [%s]", hostname.c_str());
        return OR_ResolveServerFailure;
    }
    else if (ret == EAI_NODATA)
    {
        debug("DNS lookup: hostname exists but has no ip associated [%s]", hostname.c_str());
        return OR_ResolveNoData;
    }
    else if (ret == EAI_NONAME)
    {
        debug("DNS lookup: hostname was not found [%s]", hostname.c_str());
        return OR_ResolveNotFound;
    }
    else if (ret == EAI_SYSTEM)
    {
        debug("DNS lookup: system failure, %s [%s]", strerror(errno), hostname.c_str());
        return OR_ResolveSystemFailure;
    }
    else
    {
        debug("DNS lookup: failure, %s [%s]", gai_strerror(ret), hostname.c_str());
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
cerberus::OperationResult::OperationResult(bool b1, bool b2, bool b3, bool b4)
    : res(OR_OK),
      b1(b1),
      b2(b2),
      b3(b3),
      b4(b4)
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
cerberus::OperationResult::OperationResult(SIZE s)
    : res(OR_OK),
      sz(s)
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
bool cerberus::OperationResult::ok() { return (res == Result::OR_OK); }
//=============================================================================
bool cerberus::OperationResult::fail(bool printError)
{
    if (!ok())
    {
        if (printError)
        {
            logError("Operation result failed: %s", errorString().c_str());
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
    }

    return "Undefined";
}
//=============================================================================
