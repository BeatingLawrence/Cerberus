#include "./types.h"

#ifdef WINDOWS_SYSTEM
#include <winsock2.h>
#include <ws2tcpip.h>
#ifdef ADDR_ANY
#undef ADDR_ANY
#endif
#else
#include <netdb.h>
#include <netinet/in.h>
#endif

#include <boost/regex.hpp>
#include <cstring>

#include "core/cerberusutils.h"
#include "exception/exception.h"
#include "network/socket.h"
#include "src/cerberus.h"

#ifdef WINDOWS_SYSTEM
const uint32_t crb::Host::ADDR_ANY       = 0x00000000u;
const uint32_t crb::Host::ADDR_LOOPBACK  = 0x0100007Fu;
const uint32_t crb::Host::ADDR_BROADCAST = 0xFFFFFFFFu;
#else
const uint32_t crb::Host::ADDR_ANY       = INADDR_ANY;
const uint32_t crb::Host::ADDR_LOOPBACK  = INADDR_LOOPBACK;
const uint32_t crb::Host::ADDR_BROADCAST = INADDR_BROADCAST;
#endif

//=============================================================================
crb::HASH32::HASH32(const char* str)
    : m_value(0)
{
    if (str) m_value = hashFunc_res(std::string(str));
}
//=============================================================================
crb::HASH32::HASH32(const std::string& str)
    : m_value(hashFunc_res(str))
{
}
//=============================================================================
crb::HASH32& crb::HASH32::operator=(const char* str)
{
    m_value = str ? hashFunc_res(std::string(str)) : 0u;
    return *this;
}
//=============================================================================
crb::HASH32& crb::HASH32::operator=(const std::string& str)
{
    m_value = hashFunc_res(str);
    return *this;
}
//=============================================================================
//=============================================================================
crb::Host::Host()
    : octet_networkOrder(ADDR_ANY),
      port(0),
      resolved(false)
{
}
//=============================================================================
crb::Host::Host(const std::string& str)
    : octet_networkOrder(ADDR_ANY),
      port(0),
      resolved(false)
{
    if (!fromString(str)) logError("Host %s is invalid", str.c_str());
}
//=============================================================================
crb::Host::Host(const char* str)
    : octet_networkOrder(ADDR_ANY),
      port(0),
      resolved(false)
{
    if (!fromString(str)) logError("Host %s is invalid", str);
}
//=============================================================================
crb::Host crb::Host::stringToHost(const std::string& str)
{
    Host ret{};
    ret.port = Host::getPort(str);

    std::string ip = str.substr(0, str.find_last_of(':'));
    // search for any, local or broadcast

    if (CerberusUtils::areEqual(ip, "any", WM_CaseInsensitive))
    {
        ret.octet_networkOrder = ADDR_ANY;
        return ret;
    }

    if (CerberusUtils::areEqual(ip, "local", WM_CaseInsensitive))
    {
        ret.octet_networkOrder = ADDR_LOOPBACK;
        return ret;
    }

    if (CerberusUtils::areEqual(ip, "broadcast", WM_CaseInsensitive))
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

    if (oct[0] < 0 || oct[0] > 255 || sub.empty()) return Host();

    dots[1] = ip.find_first_of('.', dots[0] + 1);
    sub     = ip.substr(dots[0] + 1, dots[1] - (dots[0] + 1));
    oct[1]  = atoi(sub.c_str());

    if (oct[1] < 0 || oct[1] > 255 || sub.empty()) return Host();

    dots[2] = ip.find_first_of('.', dots[1] + 1);
    sub     = ip.substr(dots[1] + 1, dots[2] - (dots[1] + 1));
    oct[2]  = atoi(sub.c_str());

    if (oct[2] < 0 || oct[2] > 255 || sub.empty()) return Host();

    sub    = ip.substr(dots[2] + 1);
    oct[3] = atoi(sub.c_str());

    if (oct[3] < 0 || oct[3] > 255 || sub.empty()) return Host();

    ret.octect[0] = oct[0];
    ret.octect[1] = oct[1];
    ret.octect[2] = oct[2];
    ret.octect[3] = oct[3];
    return ret;
}
//=============================================================================
crb::Host crb::Host::fromAddressPort(const std::string& address, uint16_t port)
{
    Host ret = Host::stringToHost(address);
    ret.port = port;
    return ret;
}
//=============================================================================
uint16_t crb::Host::getPort(const std::string& str)
{
    auto col    = str.find_last_of(':');
    int portint = 0;

    if (col != std::string::npos && col != str.size() - 1)
    {
        std::string portstr = str.substr(col + 1);
        portint             = atoi(portstr.c_str());

        if (portint < 0 || portint > 65535) return 0;
    }

    return portint;
}
//=============================================================================
bool crb::Host::fromString(const std::string& str)
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
std::string crb::Host::toString() const
{
    const std::string host = isTextual() ? hostname : ipToString();
    if (hasPort()) return CerberusUtils::strPrint("%s:%u", host.c_str(), port);
    return host;
}
//=============================================================================
std::string crb::Host::ipToString() const
{
    return CerberusUtils::strPrint("%u.%u.%u.%u", octect[0], octect[1], octect[2], octect[3]);
}
//=============================================================================
bool crb::Host::isValid() const { return (isNumeric() || isTextual() || hasPort()); }
//=============================================================================
bool crb::Host::isValidRemote() const { return (isNumeric() || isTextual()) && hasPort(); }
//=============================================================================
bool crb::Host::isValidNumericRemote() const { return isNumeric() && hasPort(); }
//=============================================================================
bool crb::Host::isNumeric() const { return octet_networkOrder != 0; }
//=============================================================================
bool crb::Host::isTextual() const { return !hostname.empty(); }
//=============================================================================
bool crb::Host::hasPort() const { return port != 0; }
//=============================================================================
crb::OpRes crb::Host::resolve()
{
    addrinfo* res = nullptr;
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
        sockaddr_in* addr  = (sockaddr_in*)(res->ai_addr);
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
#ifdef EAI_NODATA
    else if (ret == EAI_NODATA)
    {
        logDebug("DNS lookup: hostname exists but has no ip associated [%s]", hostname.c_str());
        return OR_ResolveNoData;
    }
#endif
    else if (ret == EAI_NONAME)
    {
        logDebug("DNS lookup: hostname was not found [%s]", hostname.c_str());
        return OR_ResolveNotFound;
    }
#ifdef EAI_SYSTEM
    else if (ret == EAI_SYSTEM)
    {
        logDebug("DNS lookup: system failure, %s [%s]", strerror(errno), hostname.c_str());
        return OR_ResolveSystemFailure;
    }
#endif
    else
    {
        logDebug("DNS lookup: failure, %s [%s]", gai_strerror(ret), hostname.c_str());
        return OR_ResolveFailure;
    }
}
//=============================================================================
crb::OpRes::OpRes()
    : res(OR_Undefined)
{
}
//=============================================================================
crb::OpRes::OpRes(Result r, const std::string& reason, const std::string& reason2)
    : res(r),
      reason(reason)
{
    addInfo(reason2);
}
//=============================================================================
crb::OpRes::OpRes(const OpRes& opres, const std::string& reason)
    : res(opres.res),
      reason(opres.reason),
      optional(opres.optional)
{
    addInfo(reason);
}
//=============================================================================
bool crb::OpRes::operator==(Result r) const { return (res == r); }
//=============================================================================
bool crb::OpRes::operator!=(Result r) const { return (res != r); }
//=============================================================================
crb::OpRes& crb::OpRes::expect(const std::string& str)
{
    if (fail()) throw cOpResExc(str.c_str());

    return *this;
}
//=============================================================================
crb::OpRes& crb::OpRes::expect(Result reason, const std::string& str)
{
    if (fail() && res == reason) throw cOpResExc(str.c_str());

    return *this;
}
//=============================================================================
crb::OpRes& crb::OpRes::expect()
{
    if (fail())
    {
        std::string errorstr = errorString();

        if (!reason.empty())
        {
            errorstr.append(", ");
            errorstr.append(reason);
        }

        throw cOpResExc(errorstr.c_str());
    }

    return *this;
}
//=============================================================================
bool crb::OpRes::ok(const std::string& str)
{
    if (res == Result::OR_OK) return true;

    if (!str.empty())
    {
        std::string err = str;
        err += ": ";
        err += errorString();

        if (!reason.empty()) err.append("\n").append(reason);

        logError("Operation failed: %s", err.c_str());
    }

    return false;
}
//=============================================================================
bool crb::OpRes::fail(const std::string& str) { return !ok(str); }
//=============================================================================
std::string crb::OpRes::errorString() const
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
        case OR_DBMissingColumns:
            return "Database missing columns";
        case OR_DBFieldSizeMismatch:
            return "Database field size mismatch";
        case OR_AlreadyPresent:
            return "Table already present";
        case OR_InvalidFile:
            return "Given file is not valid";
        case OR_NotEmpty:
            return "Item is not empty";
        case OR_Duplicate:
            return "Object is a duplicate";
        case OR_DuplicateKey:
            return "Duplicate primary key";
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
        case OR_NotEnoughData:
            return "Not enough data";
    }

    return "Undefined";
}
//=============================================================================
std::string crb::OpRes::toStr() const
{
    std::string ret;
    ret.append(errorString());
    ret.append(": ");
    ret.append(reason);
    return ret;
}
//=============================================================================
crb::OpRes& crb::OpRes::addOptional(Result opt)
{
    optional.push_back(opt);
    return *this;
}
//=============================================================================
bool crb::OpRes::hasOptional(Result opt)
{
    for (auto& el : optional)
        if (el == opt) return true;

    return false;
}
//=============================================================================
crb::LSIZE crb::OpRes::memfp() const
{
    crb::LSIZE s = reason.capacity();
    for (auto& el : optional) s += sizeof(Result);
    return s;
}
//=============================================================================
crb::OpRes& crb::OpRes::addInfo(const std::string& str)
{
    if (str.empty()) return *this;
    reason.append(", ");
    reason.append(str);
    return *this;
}
//=============================================================================
StringOpRes crb::Dictionary::getFieldValue(const std::string& key, WordMatch match) const
{
    for (auto& el : (*this))
    {
        if (CerberusUtils::areEqual(el.key, key, match)) return el.val;
    }
    return OR_NotFound;
}
//=============================================================================
crb::OpRes crb::Dictionary::getFieldMatch(const std::string& key, const std::string& value,
                                          WordMatch keymatch, WordMatch valmatch) const
{
    auto res = getFieldValue(key, keymatch);

    if (res.fail()) return res;

    if (CerberusUtils::areEqual(value, res.value, valmatch)) return OR_OK;

    return OR_Mismatch;
}
//=============================================================================
std::string crb::Dictionary::getNameAt(SIZE index) const
{
    if (index >= size()) throw cIllegalArgExc("Index out of bounds");
    return at(index).key;
}
//=============================================================================
std::string crb::Dictionary::getValueAt(SIZE index) const
{
    if (index >= size()) throw cIllegalArgExc("Index out of bounds");
    return at(index).val;
}
//=============================================================================
crb::Dictionary& crb::Dictionary::addKey(const std::string& key, const std::string& value)
{
    push_back({key, value});
    return *this;
}
//=============================================================================
crb::DictLine& crb::Dictionary::get(SIZE index)
{
    if (index >= size()) throw cIllegalArgExc("Index out of bounds");
    return at(index);
}
//=============================================================================
crb::DictLine& crb::Dictionary::get(const std::string& key, WordMatch match)
{
    for (auto it = begin(); it < end(); it++)
    {
        if (crb::CerberusUtils::areEqual((*it).key, key, match)) return (*it);
    }

    throw cIllegalArgExc("Name not found");
}
//=============================================================================
bool crb::Dictionary::exists(const std::string& key, WordMatch match)
{
    for (auto it = begin(); it < end(); it++)
    {
        if (crb::CerberusUtils::areEqual((*it).key, key, match)) return true;
    }

    return false;
}
//=============================================================================
std::string crb::Dictionary::toString() const
{
    std::string ret;

    for (auto& el : *this)
    {
        ret.append(el.key);
        ret.append(": ");
        ret.append(el.val);
        ret.append("\n");
    }

    return ret;
}
//=============================================================================
crb::LSIZE crb::Dictionary::memfp() const
{
    crb::LSIZE s = 0;
    for (auto& el : *this) s += el.sz();
    return s;
}
//=============================================================================
#if defined(LINUX_SYSTEM)
void crb::FileMetadata::fromStat(const struct statx& stat_struct)
{
    // time
    accTime.fromTimespec(stat_struct.stx_atime.tv_sec, stat_struct.stx_atime.tv_nsec);
    modTime.fromTimespec(stat_struct.stx_mtime.tv_sec, stat_struct.stx_mtime.tv_nsec);
    chgTime.fromTimespec(stat_struct.stx_ctime.tv_sec, stat_struct.stx_ctime.tv_nsec);
    creTime.fromTimespec(stat_struct.stx_btime.tv_sec, stat_struct.stx_btime.tv_nsec);

    // link refs
    linkrefs = stat_struct.stx_nlink;

    // mode
    mode.user  = 0;
    mode.group = 0;
    mode.other = 0;

    if (stat_struct.stx_mode & S_IRUSR) mode.user |= FP_READ;   // user write
    if (stat_struct.stx_mode & S_IWUSR) mode.user |= FP_WRITE;  // user read
    if (stat_struct.stx_mode & S_IXUSR) mode.user |= FP_EXEC;   // user exec
    if (stat_struct.stx_mode & S_ISUID) mode.user |= FP_UID;    // uid bit

    if (stat_struct.stx_mode & S_IRGRP) mode.group |= FP_READ;   // group read
    if (stat_struct.stx_mode & S_IWGRP) mode.group |= FP_WRITE;  // group write
    if (stat_struct.stx_mode & S_IXGRP) mode.group |= FP_EXEC;   // group exec
    if (stat_struct.stx_mode & S_ISGID) mode.user |= FP_GID;     // gid bit

    if (stat_struct.stx_mode & S_IROTH) mode.other |= FP_READ;   // other read
    if (stat_struct.stx_mode & S_IWOTH) mode.other |= FP_WRITE;  // other write
    if (stat_struct.stx_mode & S_IXOTH) mode.other |= FP_EXEC;   // other exec
    if (stat_struct.stx_mode & S_ISVTX) mode.user |= FP_STICKY;  // sticky bit

    // type
    if (stat_struct.stx_mode & S_IFBLK)
        type = FT_BLK;
    else if (stat_struct.stx_mode & S_IFCHR)
        type = FT_CHR;
    else if (stat_struct.stx_mode & S_IFDIR)
        type = FT_DIR;
    else if (stat_struct.stx_mode & S_IFIFO)
        type = FT_FIFO;
    else if (stat_struct.stx_mode & S_IFLNK)
        type = FT_LNK;
    else if (stat_struct.stx_mode & S_IFREG)
        type = FT_REG;
    else if (stat_struct.stx_mode & S_IFSOCK)
        type = FT_SOCK;

    // size
    size = stat_struct.stx_size;

    // UID, GID
    ownUID = stat_struct.stx_uid;
    ownGID = stat_struct.stx_gid;
}
#elif defined(APPLE_SYSTEM)
void crb::FileMetadata::fromStat(const struct stat& stat_struct)
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
#endif
//=============================================================================
crb::SocketCloser::~SocketCloser()
{
    if (socket) socket->close();
}
//=============================================================================
namespace
{
    const char* kOpaqueIntPattern    = "[-+]?\\d+";
    const char* kOpaqueDoublePattern = "[-+]?(\\d+\\.\\d+|\\d+\\.|\\.\\d+)";
    const char* kOpaqueBoolPattern   = "(true|false)";

    inline bool matches_pattern(const std::string& value, const char* pattern)
    {
        static const boost::regex intRegex(
            kOpaqueIntPattern, boost::regex::ECMAScript | boost::regex::icase | boost::regex::optimize);
        static const boost::regex doubleRegex(
            kOpaqueDoublePattern, boost::regex::ECMAScript | boost::regex::icase | boost::regex::optimize);
        static const boost::regex boolRegex(
            kOpaqueBoolPattern, boost::regex::ECMAScript | boost::regex::icase | boost::regex::optimize);

        const boost::regex* rx = nullptr;
        if (std::strcmp(pattern, kOpaqueIntPattern) == 0)
            rx = &intRegex;
        else if (std::strcmp(pattern, kOpaqueDoublePattern) == 0)
            rx = &doubleRegex;
        else
            rx = &boolRegex;

        boost::smatch m;
        return boost::regex_match(value, m, *rx);
    }
}  // namespace
//=============================================================================
crb::Opaque::Opaque(const char* str)
    : value(str ? str : "")
{
}
//=============================================================================
crb::Opaque::Opaque(const std::string& str)
    : value(str)
{
}
//=============================================================================
crb::Opaque::Opaque(int val) { setInt(val); }
//=============================================================================
crb::Opaque::Opaque(unsigned int val) { setUInt(val); }
//=============================================================================
crb::Opaque::Opaque(int64_t val) { setInt(val); }
//=============================================================================
crb::Opaque::Opaque(uint64_t val) { setUInt(val); }
//=============================================================================
crb::Opaque::Opaque(double val) { setDouble(val); }
//=============================================================================
crb::Opaque::Opaque(float val) { setDouble(static_cast<double>(val)); }
//=============================================================================
crb::Opaque::Opaque(bool val) { setBool(val); }
//=============================================================================
crb::DataType crb::Opaque::type() const
{
    if (value.empty()) return DT_Invalid;

    // Priority: double > integer > bool
    if (matches_pattern(value, kOpaqueDoublePattern)) return DT_Double;
    if (matches_pattern(value, kOpaqueIntPattern)) return DT_Integer;
    if (matches_pattern(value, kOpaqueBoolPattern)) return DT_Bool;

    return DT_Invalid;  // treat as string/other
}
//=============================================================================
const std::string& crb::Opaque::get() const { return value; }
//=============================================================================
void crb::Opaque::set(const std::string& str) { value = str; }
//=============================================================================
void crb::Opaque::setInt(int64_t val) { value = CerberusUtils::strPrint_int(val); }
//=============================================================================
void crb::Opaque::setUInt(uint64_t val) { value = CerberusUtils::strPrint("%llu", (unsigned long long)val); }
//=============================================================================
void crb::Opaque::setDouble(double val)
{
    value = CerberusUtils::strPrint_float(static_cast<long double>(val));
    CerberusUtils::cleanNumber(value);  // normalize representation (e.g., remove trailing zeros)
}
//=============================================================================
void crb::Opaque::setBool(bool val) { value = val ? "true" : "false"; }
//=============================================================================
crb::Opaque& crb::Opaque::operator=(const std::string& str)
{
    set(str);
    return *this;
}
//=============================================================================
int64_t crb::Opaque::getInt()
{
    if (type() != DT_Integer) throw cInvalidCastExc("Opaque value \"%s\" is not an integer", value.c_str());

    auto res = CerberusUtils::stringToInt(value);
    if (res.fail()) throw cInvalidCastExc("Failed to convert \"%s\" to integer", value.c_str());
    return res.value;
}
//=============================================================================
double crb::Opaque::getDouble()
{
    auto t = type();
    if (t != DT_Double && t != DT_Integer)
        throw cInvalidCastExc("Opaque value \"%s\" is not a double-compatible value", value.c_str());

    if (t == DT_Double)
    {
        auto res = CerberusUtils::stringToDouble(value);
        if (res.fail()) throw cInvalidCastExc("Failed to convert \"%s\" to double", value.c_str());
        return static_cast<double>(res.value);
    }

    // Integer string is convertible to double as a convenience.
    auto res = CerberusUtils::stringToInt(value);
    if (res.fail()) throw cInvalidCastExc("Failed to convert \"%s\" to double", value.c_str());
    return static_cast<double>(res.value);
}
//=============================================================================
bool crb::Opaque::getBool()
{
    if (type() != DT_Bool) throw cInvalidCastExc("Opaque value \"%s\" is not a boolean", value.c_str());
    return CerberusUtils::areEqual(value, "true", WM_CaseInsensitive);
}
//=============================================================================
