#include "cerberusutils.h"

#include <inttypes.h>
#if (defined(__GNUG__) || defined(__clang__)) && !defined(_MSC_VER)
#include <cxxabi.h>
#endif
#if defined(WINDOWS_SYSTEM)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#elif defined(LINUX_SYSTEM)
#include <sys/sysinfo.h>
#elif defined(APPLE_SYSTEM)
#include <sys/sysctl.h>
#endif

#include <algorithm>
#include <boost/regex.hpp>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <limits>

#include "../cerberus.h"              // IWYU pragma: export
#include "../data/filesystem/file.h"  // IWYU pragma: export
#include "../message/slot.h"          // IWYU pragma: export

#define FNV_OFFSET_BASIS 0x811c9dc5
#define FNV_PRIME 0x01000193

using namespace crb;

namespace
{
bool isEdgeBlank(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}
}  // namespace

std::regex CerberusUtils::isNumberRegex("\\-?[0-9]+(?:\\.[0-9]+)?", std::regex_constants::ECMAScript |
                                                                        std::regex_constants::optimize |
                                                                        std::regex_constants::icase);

//=============================================================================
std::string CerberusUtils::demangleTypeName(const char* name)
{
    if (!name) return std::string();

#if (defined(__GNUG__) || defined(__clang__)) && !defined(_MSC_VER)
    int status = 0;
    char* demangled = abi::__cxa_demangle(name, nullptr, nullptr, &status);
    std::string ret = (status == 0 && demangled) ? demangled : name;
    free(demangled);
    return ret;
#else
    std::string ret(name);
    const char* prefixes[] = {"class ", "struct ", "enum "};
    for (const char* prefix : prefixes)
    {
        size_t len = strlen(prefix);
        if (ret.compare(0, len, prefix) == 0)
        {
            ret.erase(0, len);
            break;
        }
    }
    return ret;
#endif
}

//=============================================================================
std::string CerberusUtils::strPrint(std::string format, ...)
{
    std::string ret;
    if (format.empty()) return ret;
    va_list testList;
    va_list list;
    va_start(testList, format);
    va_copy(list, testList);
    char garbage;
    int required = vsnprintf(&garbage, 0, format.c_str(), testList);

    if (required > 0)
    {
        ret.resize(static_cast<size_t>(required) + 1u);
        vsnprintf(&ret[0], ret.size(), format.c_str(), list);
        ret.resize(static_cast<size_t>(required));
    }

    va_end(testList);
    va_end(list);
    return ret;
}
//=============================================================================
std::string CerberusUtils::strPrint_valist(std::string format, va_list list)
{
    std::string ret;
    if (format.empty()) return ret;
    va_list testList;
    va_copy(testList, list);
    char garbage;
    int required = vsnprintf(&garbage, 0, format.c_str(), testList);

    if (required > 0)
    {
        ret.resize(static_cast<size_t>(required) + 1u);
        vsnprintf(&ret[0], ret.size(), format.c_str(), list);
        ret.resize(static_cast<size_t>(required));
    }

    va_end(testList);
    va_end(list);
    return ret;
}
//=============================================================================
std::string CerberusUtils::strPrint_uint(uint64_t v) { return strPrint("%" PRIu64, v); }
//=============================================================================
std::string CerberusUtils::strPrint_int(int64_t v) { return strPrint("%" PRIi64, v); }
//=============================================================================
std::string CerberusUtils::strPrint_float(long double v) { return strPrint("%Lf", v); }
//=============================================================================
std::string CerberusUtils::toLower(const std::string& str)
{
    std::string data(str);
    std::transform(data.begin(), data.end(), data.begin(), [](unsigned char c) { return std::tolower(c); });
    return data;
}
//=============================================================================
std::string CerberusUtils::toUpper(const std::string& str)
{
    std::string data(str);
    std::transform(data.begin(), data.end(), data.begin(), [](unsigned char c) { return std::toupper(c); });
    return data;
}
//=============================================================================
std::string CerberusUtils::removeBlankBefore(const std::string& str)
{
    size_t pos = 0;
    for (auto&& el : str)
    {
        if (!isEdgeBlank(el)) break;
        pos++;
    }

    return str.substr(pos);
}
//=============================================================================
std::string CerberusUtils::removeBlankAfter(const std::string& str)
{
    size_t len = str.size();
    for (auto it = str.rbegin(); it != str.rend(); it++)
    {
        if (!isEdgeBlank(*it)) break;
        len--;
    }

    return str.substr(0, len);
}
//=============================================================================
std::string CerberusUtils::removeBlank_copy(const std::string& str)
{
    return removeBlankAfter(removeBlankBefore(str));
}
//=============================================================================
void CerberusUtils::removeBlank(std::string& str)
{
    str = removeBlankBefore(str);
    str = removeBlankAfter(str);
}
//=============================================================================
bool CerberusUtils::contains(const std::string& str1, const std::string& str2)
{
    if (str1.find(str2) == std::string::npos) return false;

    return true;
}
//=============================================================================
bool CerberusUtils::contains(const std::string& str1, char c)
{
    if (str1.find(c) == std::string::npos) return false;

    return true;
}
//=============================================================================
bool CerberusUtils::areEqual(const std::string& str1, const std::string& str2, WordMatch match)
{
    switch (match)
    {
        case WM_CaseSensitive:
            if (str1.compare(str2) == 0)
            {
                return true;
            }
            break;

        case WM_CaseInsensitive:
            auto s1 = toLower(str1);
            auto s2 = toLower(str2);

            if (s1.compare(s2) == 0)
            {
                return true;
            }
            break;
    }

    return false;
}
//=============================================================================
StringOpRes CerberusUtils::environmentVariable(const std::string& variableName)
{
    char* val = getenv(variableName.c_str());

    if (val == nullptr)
    {
        return OR_NotFound;
    }

    return std::string(val);
}
//=============================================================================
IntOpRes CerberusUtils::stringToInt(const std::string& str, Radix r)
{
    int64_t ret = 0;

    try
    {
        ret = std::stoll(str, nullptr,
                         r == Radix::Binary        ? 2
                         : r == Radix::Decimal     ? 10
                         : r == Radix::Hexadecimal ? 16
                                                   : 0);
    }
    catch (...)
    {
        return OR_WrongArgument;
    }

    return ret;
}
//=============================================================================
FloatOpRes CerberusUtils::stringToDouble(const std::string& str)
{
    long double ret = 0.0f;

    try
    {
        ret = std::stold(str, nullptr);
    }
    catch (...)
    {
        return OR_WrongArgument;
    }

    return ret;
}
//=============================================================================
bool CerberusUtils::isAlpha(const std::string& str)
{
    for (auto&& ch : str)
        if ((ch >= 65 && ch <= 90) || (ch >= 97 && ch <= 122))
        {
            return true;
        }

    return false;
}
//=============================================================================
bool CerberusUtils::isNumber(const std::string& str) { return std::regex_match(str, isNumberRegex); }
//=============================================================================
bool CerberusUtils::isBool(const std::string& str, WordMatch match)
{
    return (CerberusUtils::areEqual(str, "true", match) || CerberusUtils::areEqual(str, "false", match));
}
//=============================================================================
bool CerberusUtils::startsWith(const std::string& str1, const std::string& str2)
{
    if (str2.size() > str1.size()) return false;

    for (SIZE i = 0; i < str2.size(); i++)
        if (str1[i] != str2[i])
        {
            return false;
        }

    return true;
}
//=============================================================================
bool CerberusUtils::startsWith(const std::string& str, char c)
{
    if (str.empty()) return false;
    return (str.front() == c);
}
//=============================================================================
bool CerberusUtils::endsWith(const std::string& str1, const std::string& str2)
{
    if (str2.size() > str1.size()) return false;

    SIZE diff = static_cast<SIZE>(str1.size() - str2.size());

    for (SIZE i = 0; i < str2.size(); i++)
        if (str1[diff + i] != str2[i])
        {
            return false;
        }

    return true;
}
//=============================================================================
bool CerberusUtils::endsWith(const std::string& str, char c)
{
    if (str.empty()) return false;
    return (str.back() == c);
}
//=============================================================================
OpRes CerberusUtils::patternMatch(const std::string& str, const std::string& pattern)
{
    if (str.empty()) return OR_Empty;

    try
    {
        boost::regex r(pattern);
        boost::smatch m;

        if (boost::regex_match(str, m, r)) return OR_OK;
    }
    catch (std::exception& e)
    {
        return {OR_Failure, e.what()};
    }

    return OR_NotFound;
}
//=============================================================================
void CerberusUtils::replaceAll(std::string& str, const std::string& find, const std::string& replace)
{
    size_t start = 0;

    while ((start = str.find(find, start)) != std::string::npos)
    {
        str.replace(start, find.length(), replace);
        start += replace.length();
    }
}
//=============================================================================
StringOpRes CerberusUtils::replaceAll_regex(const std::string& str, const std::string& pattern,
                                            const std::string& replace)
{
    if (str.empty()) return OR_Empty;

    try
    {
        boost::regex r(pattern);
        return boost::regex_replace(str, r, replace);
    }
    catch (std::exception& e)
    {
        return {OR_Failure, e.what()};
    }
}
//=============================================================================
bool CerberusUtils::normalize(std::string& str)
{
    bool ret = false;

    CerberusUtils::replaceAll(str, "\r", "\\r");
    CerberusUtils::replaceAll(str, "\n", "\\n\n");

    for (auto&& el : str)
        if ((el < 32 || el > 126) && el != '\n' && el != 0)
        {
            el  = '#';
            ret = true;
        }

    return ret;
}
//=============================================================================
std::string CerberusUtils::hex(const ByteBuffer& buffer)
{
    std::string ret;

    for (auto& el : buffer) ret.append(strPrint("%02hhx", (uint8_t)el));

    return ret;
}
//=============================================================================
std::string CerberusUtils::truncStr(const std::string& str, crb::SIZE size)
{
    if (size >= str.size())
    {
        return str;
    }

    return str.substr(0, size);
}
//=============================================================================
std::string CerberusUtils::substrUntil(const std::string& str, const std::string& pattern)
{
    auto found = str.find(pattern);

    if (found == std::string::npos) return str;

    return str.substr(0, found);
}
//=============================================================================
std::string CerberusUtils::substrFrom(const std::string& str, const std::string& pattern)
{
    auto found = str.find(pattern);

    if (found == std::string::npos) return "";

    return str.substr(found + pattern.size());
}
//=============================================================================
StringOpRes CerberusUtils::substrUntil_regex(const std::string& str, const std::string& pattern, bool invert)
{
    if (str.empty()) return OR_Empty;

    try
    {
        boost::regex r(pattern);
        boost::smatch m;

        if (boost::regex_search(str, m, r))
        {
            // at least one match found

            const size_t matchIndex64 = invert ? m.size() - 1 : 0;
            if (matchIndex64 > static_cast<size_t>(std::numeric_limits<int>::max()))
                return {OR_Failure, "regex match index too large"};

            const int matchIndex = static_cast<int>(matchIndex64);
            size_t pos64 = static_cast<size_t>(m[matchIndex].first - str.begin());
            if (pos64 > std::numeric_limits<SIZE>::max()) return {OR_Failure, "regex match position too large"};

            SIZE pos = static_cast<SIZE>(pos64);
            return str.substr(0, pos);
        }
    }
    catch (std::exception& e)
    {
        return {OR_Failure, e.what()};
    }

    return OR_NotFound;
}
//=============================================================================
StringOpRes CerberusUtils::substrFrom_regex(const std::string& str, const std::string& pattern, bool invert)
{
    if (str.empty()) return OR_Empty;

    try
    {
        boost::regex r(pattern);
        boost::smatch m;

        if (boost::regex_search(str, m, r))
        {
            // at least one match found

            const size_t matchIndex64 = invert ? m.size() - 1 : 0;
            if (matchIndex64 > static_cast<size_t>(std::numeric_limits<int>::max()))
                return {OR_Failure, "regex match index too large"};

            const int matchIndex = static_cast<int>(matchIndex64);
            size_t pos64 = static_cast<size_t>(m[matchIndex].second - str.begin());
            if (pos64 > std::numeric_limits<SIZE>::max()) return {OR_Failure, "regex match position too large"};

            SIZE pos = static_cast<SIZE>(pos64);
            return str.substr(pos);
        }
    }
    catch (std::exception& e)
    {
        return {OR_Failure, e.what()};
    }

    return OR_NotFound;
}
//=============================================================================
DoubleString CerberusUtils::split(const std::string& str, const std::string& token)
{
    return {substrUntil(str, token), substrFrom(str, token)};
}
//=============================================================================
OpRes CerberusUtils::cleanNumber(std::string& str)
{
    if (!isNumber(str)) return {OR_WrongArgument, strPrint("Given %s is not a number", str.c_str())};

    bool negative = false;

    if (startsWith(str, '-'))
    {
        // negative number
        str.erase(0, 1);
        negative = true;
    }

    bool isFloating = contains(str, '.');

    if (isFloating)
    {
        // remove leading zeros

        auto pos = str.find_first_not_of('0');  // cannot be npos

        if (pos != 0)
        {
            if (str[pos] == '.' && str[pos - 1] == '0') pos--;

            str = str.substr(pos);
        }

        // remove trailing zeros

        pos = str.find_last_not_of('0');  // cannot be npos

        if (str[pos] == '.')
            str = str.substr(0, pos);  // remove also the dot
        else
            str = str.substr(0, pos + 1);
    }
    else
    {
        // remove leading zeros
        auto pos = str.find_first_not_of('0');

        if (pos == std::string::npos)
            str = '0';
        else
            str = str.substr(pos);
    }

    if (negative && !areEqual(str, "0") && !areEqual(str, "0.0")) str.insert(str.begin(), '-');

    return OR_OK;
}
//=============================================================================
HASH32 CerberusUtils::hash_fnv1a(const std::string& str)
{
    // This method uses FNV1A algorithm
    HASH32 hash    = FNV_OFFSET_BASIS;
    const char* ch = str.c_str();

    while ((*ch) != '\0')
    {
        hash = hash ^ (HASH32)(*ch);
        hash = hash * FNV_PRIME;
        ch++;
    }

    return hash;
}
//=============================================================================
HASH32 CerberusUtils::hash_fnv1a(const ByteBuffer& buf)
{
    // This method uses FNV1A algorithm
    HASH32 hash   = FNV_OFFSET_BASIS;
    const BYTE* b = buf.data();

    for (crb::LSIZE i = 0; i < buf.size(); i++)
    {
        hash = hash ^ (HASH32)(*b);
        hash = hash * FNV_PRIME;
        b++;
    }

    return hash;
}
//=============================================================================
StringOpRes CerberusUtils::completePath(const std::string& path)
{
#ifdef WINDOWS_SYSTEM
    std::error_code ec;
    auto comp = std::filesystem::canonical(path, ec);
    if (ec) return OR_Failure;
    return comp.string();
#else
    auto comp = ::realpath(path.c_str(), NULL);

    if (comp == NULL) return OR_Failure;

    std::string ret(comp);
    free(comp);
    return ret;
#endif
}
//=============================================================================
DBDataType CerberusUtils::toDBDataType(const std::string& type)
{
    if (type.compare("bigint") == 0)
        return DBDataType::DDT_BigInt;

    else if (type.compare("integer") == 0)
        return DBDataType::DDT_Int;

    else if (type.compare("smallint") == 0)
        return DBDataType::DDT_SmallInt;

    else if (type.compare("real") == 0)
        return DBDataType::DDT_Real;

    else if (type.compare("double precision") == 0)
        return DBDataType::DDT_Double;

    else if (type.compare("boolean") == 0)
        return DBDataType::DDT_Boolean;

    else if (type.compare("money") == 0)
        return DBDataType::DDT_Money;

    else if (CerberusUtils::contains(type, "character"))
    {
        if (CerberusUtils::contains(type, "varying"))
            return DBDataType::DDT_VarChar;

        else
            return DBDataType::DDT_Char;
    }
    else if (CerberusUtils::contains(type, "bit"))
    {
        if (CerberusUtils::contains(type, "varying"))
            return DBDataType::DDT_VarBit;

        else
            return DBDataType::DDT_Bit;
    }

    return DBDataType::DDT_Undefined;
}
//=============================================================================
std::string CerberusUtils::fromDBDataType(DBDataType type)
{
    switch (type)
    {
        case DBDataType::DDT_Undefined:
            return "";

        case DBDataType::DDT_Int:
            return "int";

        case DBDataType::DDT_SmallInt:
            return "sint";

        case DBDataType::DDT_BigInt:
            return "bint";

        case DBDataType::DDT_Real:
            return "real";

        case DBDataType::DDT_Double:
            return "double";

        case DBDataType::DDT_Boolean:
            return "bool";

        case DBDataType::DDT_Bit:
            return "bit";

        case DBDataType::DDT_VarBit:
            return "varbit";

        case DBDataType::DDT_Char:
            return "char";

        case DBDataType::DDT_VarChar:
            return "varchar";

        case DBDataType::DDT_Money:
            return "money";
    }

    return "";
}
//=============================================================================
LSIZE CerberusUtils::qceil(LSIZE dividend, LSIZE divisor)
{
    return dividend / divisor + (dividend % divisor != 0);
}
//=============================================================================
uint8_t CerberusUtils::reqBytes(LSIZE num)
{
    if (!num) return 1;

    LSIZE mask = (LSIZE)0xFF << 56;

    for (uint8_t i = 8; i != 0; i--)
    {
        if (num & mask) return i;
        mask >>= 8;
    }

    return 1;
}
//=============================================================================
OpResData<CoreSet> CerberusUtils::getOnlineCoreSet()
{
    int cores = 0;
#if defined(WINDOWS_SYSTEM)
    DWORD activeCores = GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
    if (activeCores == 0 || activeCores > static_cast<DWORD>(std::numeric_limits<int>::max()))
        return OpResData<CoreSet>(OR_Failure, "GetActiveProcessorCount fail");
    cores = static_cast<int>(activeCores);
#elif !defined(APPLE_SYSTEM)
    cores = get_nprocs();
    if (cores <= 0) return OpResData<CoreSet>(OR_Failure, "get_nprocs fail");
#else
    size_t len = sizeof(cores);
    if (sysctlbyname("hw.logicalcpu", &cores, &len, nullptr, 0) != 0 || cores <= 0)
        return OpResData<CoreSet>(OR_Failure, "sysctl hw.logicalcpu fail");
#endif

    CoreSet set;
    set.cores.reserve(static_cast<size_t>(cores));

    for (int i = 0; i < cores; ++i) set.addCore(i);

    return set;
}
//=============================================================================
OpRes CerberusUtils::assignIRQ(int irq, const CoreSet& coresIn)
{
    if (irq < 0) return OR_WrongArgument;
    if (coresIn.cores.empty()) return OR_WrongArgument;

    std::vector<int> cores = coresIn.cores;
    std::sort(cores.begin(), cores.end());
    cores.erase(std::unique(cores.begin(), cores.end()), cores.end());

    std::string list;
    for (size_t i = 0; i < cores.size(); ++i)
    {
        if (i) list.append(",");
        list.append(strPrint("%d", cores[i]));
    }

    if (list.empty()) return OR_WrongArgument;

    std::string path = strPrint("/proc/irq/%d/smp_affinity_list", irq);
    File f(path, FOM_ReadWrite);

    auto r = f.open();
    if (r.fail()) return r;

    ByteBuffer data(list);
    auto w = f.write(data);
    f.close();

    return w;
}
//=============================================================================
