#include "./types.h"

#include <netdb.h>

#include <cstring>

#include "core/cerberusutils.h"
#include "exception/exception.h"
#include "src/cerberus.h"

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
    if (!fromString(str)) logError("Host %s is invalid", str.c_str());
}
//=============================================================================
cerberus::Host::Host(const char *str)
    : octet_networkOrder(ADDR_ANY),
      port(0),
      resolved(false)
{
    if (!fromString(str)) logError("Host %s is invalid", str);
}
//=============================================================================
cerberus::Host cerberus::Host::stringToHost(const std::string &str)
{
    Host ret{};
    ret.port = Host::getPort(str);

    std::string ip = str.substr(0, str.find_last_of(':'));
    // search for any, local or broadcast

    if (CerberusUtils::areEqual(CerberusUtils::toLower(ip), "any"))
    {
        ret.octet_networkOrder = ADDR_ANY;
        return ret;
    }

    if (CerberusUtils::areEqual(CerberusUtils::toLower(ip), "local"))
    {
        ret.octet_networkOrder = ADDR_LOOPBACK;
        return ret;
    }

    if (CerberusUtils::areEqual(CerberusUtils::toLower(ip), "broadcast"))
    {
        ret.octet_networkOrder = ADDR_BROADCAST;
        return ret;
    }

    if (CerberusUtils::isAlpha(ip))  // Domain name
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
std::string cerberus::Host::toString() const
{
    return CerberusUtils::strPrint("%u.%u.%u.%u:%u", octect[0], octect[1], octect[2], octect[3], port);
}
//=============================================================================
bool cerberus::Host::isValid() const { return (isNumeric() || isTextual() || hasPort()); }
//=============================================================================
bool cerberus::Host::isValidRemote() const { return (isNumeric() || isTextual()) && hasPort(); }
//=============================================================================
bool cerberus::Host::isNumeric() const { return octet_networkOrder != 0; }
//=============================================================================
bool cerberus::Host::isTextual() const { return !hostname.empty(); }
//=============================================================================
bool cerberus::Host::hasPort() const { return port != 0; }
//=============================================================================
cerberus::OpRes cerberus::Host::resolve()
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
        logDebug("DNS lookup: temporary server failure [%s]", hostname.c_str());
        return OR_ResolveServerTempFailure;
    }
    else if (ret == EAI_FAIL)
    {
        logDebug("DNS lookup: server failure [%s]", hostname.c_str());
        return OR_ResolveServerFailure;
    }
    else if (ret == EAI_NODATA)
    {
        logDebug("DNS lookup: hostname exists but has no ip associated [%s]", hostname.c_str());
        return OR_ResolveNoData;
    }
    else if (ret == EAI_NONAME)
    {
        logDebug("DNS lookup: hostname was not found [%s]", hostname.c_str());
        return OR_ResolveNotFound;
    }
    else if (ret == EAI_SYSTEM)
    {
        logDebug("DNS lookup: system failure, %s [%s]", strerror(errno), hostname.c_str());
        return OR_ResolveSystemFailure;
    }
    else
    {
        logDebug("DNS lookup: failure, %s [%s]", gai_strerror(ret), hostname.c_str());
        return OR_ResolveFailure;
    }
}
//=============================================================================
cerberus::OpRes::OpRes()
    : res(OR_Undefined)
{
}
//=============================================================================
cerberus::OpRes::OpRes(Result r, const std::string &reason, const std::string &reason2)
    : res(r),
      reason(reason + " " + reason2)
{
}
//=============================================================================
bool cerberus::OpRes::operator==(Result r) { return (res == r); }
//=============================================================================
bool cerberus::OpRes::operator!=(Result r) { return (res != r); }
//=============================================================================
cerberus::OpRes &cerberus::OpRes::expect(const std::string &str)
{
    if (fail()) throw cerberusOpResExc(str.c_str());

    return *this;
}
//=============================================================================
cerberus::OpRes &cerberus::OpRes::expect(Result reason, const std::string &str)
{
    if (fail() && res == reason) throw cerberusOpResExc(str.c_str());

    return *this;
}
//=============================================================================
cerberus::OpRes &cerberus::OpRes::expect()
{
    if (fail())
    {
        std::string errorstr = errorString();

        if (!reason.empty())
        {
            errorstr.append(", ");
            errorstr.append(reason);
        }

        throw cerberusOpResExc(errorstr.c_str());
    }

    return *this;
}
//=============================================================================
bool cerberus::OpRes::ok(const std::string &str)
{
    if (res == Result::OR_OK)
    {
        return true;
    }
    else if (!str.empty())
    {
        std::string err = str;
        err += " ,";
        err += errorString();

        if (!reason.empty()) err.append("\n").append(reason);

        logError("Operation failed: %s", err.c_str());
    }

    return false;
}
//=============================================================================
bool cerberus::OpRes::fail(const std::string &str) { return !ok(str); }
//=============================================================================
std::string cerberus::OpRes::errorString()
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
            return "Bad conditions";
        case OR_ResolveServerTempFailure:
            return "The name server returned a temporary failure indication";
        case OR_ResolveServerFailure:
            return "The name server returned a failure indication";
        case OR_ResolveNoData:
            return "The host exists, but does not have any network addresses defined";
        case OR_ResolveNotFound:
            return "The node or service is not known";
        case OR_ResolveSystemFailure:
            return "System error";
        case OR_ResolveFailure:
            return "Generic resolve failure";
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
        case OR_NotEmpty:
            return "Item is not empty";
        case OR_Duplicate:
            return "Object is a duplicate";
        case OR_EOF:
            return "End of file";
        case OR_WrongType:
            return "Wrong type";
        case OR_ThreadNotJoinable:
            return "Thread not joinable";
        case OR_Empty:
            return "Item is empty";
        case OR_Mismatch:
            return "Item mismatch";
        case OR_TLSKeysCheckFail:
            return "TLS key check failure";
        case OR_WrongData:
            return "Wrong data";
    }

    return "Undefined";
}
//=============================================================================
cerberus::OpRes &cerberus::OpRes::addOptional(Result opt)
{
    optional.push_back(opt);
    return *this;
}
//=============================================================================
bool cerberus::OpRes::hasOptional(Result opt)
{
    for (auto &el : optional)
    {
        if (el == opt) return true;
    }

    return false;
}
//=============================================================================
StringOpRes cerberus::Dictionary::getFieldValue(const std::string &key, WordMatch match) const
{
    for (auto &el : (*this))
    {
        if (CerberusUtils::areEqual(el.key, key, match)) return el.val;
    }
    return OR_NotFound;
}
//=============================================================================
cerberus::OpRes cerberus::Dictionary::getFieldMatch(const std::string &key, const std::string &value,
                                                    WordMatch keymatch, WordMatch valmatch) const
{
    auto res = getFieldValue(key, keymatch);

    if (res.fail()) return res;

    if (CerberusUtils::areEqual(value, res.value, valmatch)) return OR_OK;

    return OR_Mismatch;
}
//=============================================================================
std::string cerberus::Dictionary::getNameAt(SIZE index) const
{
    if (index >= size()) throw cerberusIllegalArgExc("Index out of bounds");
    return at(index).key;
}
//=============================================================================
std::string cerberus::Dictionary::getValueAt(SIZE index) const
{
    if (index >= size()) throw cerberusIllegalArgExc("Index out of bounds");
    return at(index).val;
}
//=============================================================================
cerberus::Dictionary &cerberus::Dictionary::addKey(const std::string &key, const std::string &value)
{
    push_back({key, value});
    return *this;
}
//=============================================================================
cerberus::DictLine &cerberus::Dictionary::get(SIZE index)
{
    if (index >= size()) throw cerberusIllegalArgExc("Index out of bounds");
    return at(index);
}
//=============================================================================
cerberus::DictLine &cerberus::Dictionary::get(const std::string &key, WordMatch match)
{
    for (auto it = begin(); it < end(); it++)
    {
        if (cerberus::CerberusUtils::areEqual((*it).key, key, match)) return (*it);
    }

    throw cerberusIllegalArgExc("Name not found");
}
//=============================================================================
bool cerberus::Dictionary::exists(const std::string &key, WordMatch match)
{
    for (auto it = begin(); it < end(); it++)
    {
        if (cerberus::CerberusUtils::areEqual((*it).key, key, match)) return true;
    }

    return false;
}
//=============================================================================
void cerberus::FileMetadata::fromStat(const struct stat &stat_struct)
{
    // time
    accTime.fromTimespec(stat_struct.st_atimespec.tv_sec, stat_struct.st_atimespec.tv_nsec);
    modTime.fromTimespec(stat_struct.st_mtimespec.tv_sec, stat_struct.st_mtimespec.tv_nsec);
    chgTime.fromTimespec(stat_struct.st_ctimespec.tv_sec, stat_struct.st_ctimespec.tv_nsec);
    creTime.fromTimespec(stat_struct.st_birthtimespec.tv_sec, stat_struct.st_birthtimespec.tv_nsec);

    // link refs
    linkrefs = stat_struct.st_nlink;

    // mode
    mode.user  = 0;
    mode.group = 0;
    mode.other = 0;

    if (stat_struct.st_mode & S_IRUSR) mode.user |= FP_READ;   // user write
    if (stat_struct.st_mode & S_IWUSR) mode.user |= FP_WRITE;  // user read
    if (stat_struct.st_mode & S_IXUSR) mode.user |= FP_EXEC;   // user exec
    if (stat_struct.st_mode & S_ISUID) mode.user |= FP_UID;    // uid bit

    if (stat_struct.st_mode & S_IRGRP) mode.group |= FP_READ;   // group read
    if (stat_struct.st_mode & S_IWGRP) mode.group |= FP_WRITE;  // group write
    if (stat_struct.st_mode & S_IXGRP) mode.group |= FP_EXEC;   // group exec
    if (stat_struct.st_mode & S_ISGID) mode.user |= FP_GID;     // gid bit

    if (stat_struct.st_mode & S_IROTH) mode.other |= FP_READ;   // other read
    if (stat_struct.st_mode & S_IWOTH) mode.other |= FP_WRITE;  // other write
    if (stat_struct.st_mode & S_IXOTH) mode.other |= FP_EXEC;   // other exec
    if (stat_struct.st_mode & S_ISVTX) mode.user |= FP_STICKY;  // sticky bit

    // type
    if (stat_struct.st_mode & S_IFBLK)
        type = FT_BLK;
    else if (stat_struct.st_mode & S_IFCHR)
        type = FT_CHR;
    else if (stat_struct.st_mode & S_IFDIR)
        type = FT_DIR;
    else if (stat_struct.st_mode & S_IFIFO)
        type = FT_FIFO;
    else if (stat_struct.st_mode & S_IFLNK)
        type = FT_LNK;
    else if (stat_struct.st_mode & S_IFREG)
        type = FT_REG;
    else if (stat_struct.st_mode & S_IFSOCK)
        type = FT_SOCK;

    // size
    size = stat_struct.st_size;

    // UID, GID
    ownUID = stat_struct.st_uid;
    ownGID = stat_struct.st_gid;
}
//=============================================================================
