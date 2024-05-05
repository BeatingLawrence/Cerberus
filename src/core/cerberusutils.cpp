#include "cerberusutils.h"

#include <algorithm>
#include <cstdarg>
#include <cstdlib>
#include <cstring>

#include "src/cerberus.h"
#include "src/message/slot.h"

#define FNV_OFFSET_BASIS 0x811c9dc5
#define FNV_PRIME 0x01000193

using namespace cerberus;

std::regex CerberusUtils::isNumberRegex("\\-?[0-9]+(?:\\.[0-9]+)?", std::regex_constants::ECMAScript |
                                                                        std::regex_constants::optimize |
                                                                        std::regex_constants::icase);

//=============================================================================
std::string CerberusUtils::strPrint(std::string format, ...)
{
    std::string ret;
    va_list testList;
    va_list list;
    va_start(testList, format);
    va_copy(list, testList);
    char garbage;
    int required = vsnprintf(&garbage, 0, format.c_str(), testList);

    if (required > 0)
    {
        ret.resize(required);
        vsnprintf(&ret[0], required + 1, format.c_str(), list);
    }

    va_end(testList);
    va_end(list);
    return ret;
}
//=============================================================================
std::string CerberusUtils::strPrint_valist(std::string format, va_list list)
{
    std::string ret;
    va_list testList;
    va_copy(testList, list);
    char garbage;
    int required = vsnprintf(&garbage, 0, format.c_str(), testList);

    if (required > 0)
    {
        ret.resize(required);
        vsnprintf(&ret[0], required + 1, format.c_str(), list);
    }

    va_end(testList);
    va_end(list);
    return ret;
}
//=============================================================================
std::string CerberusUtils::strPrint_uint(uint64_t v) { return strPrint("%lli", v); }
//=============================================================================
std::string CerberusUtils::strPrint_int(int64_t v) { return strPrint("%llu", v); }
//=============================================================================
std::string CerberusUtils::strPrint_float(double v) { return strPrint("%Lf", v); }
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
        if (el != ' ' && el != 0x9) break;  // space or TAB
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
        if (*it != ' ' && *it != 0x9) break;  // space or TAB
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
    {
        if ((ch >= 65 && ch <= 90) || (ch >= 97 && ch <= 122))
        {
            return true;
        }
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

    for (int i = 0; i < str2.size(); i++)
    {
        if (str1[i] != str2[i])
        {
            return false;
        }
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

    int diff = str1.size() - str2.size();

    for (int i = 0; i < str2.size(); i++)
    {
        if (str1[diff + i] != str2[i])
        {
            return false;
        }
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
void CerberusUtils::replaceAll(std::string& str, const std::string& find, const std::string& replace)
{
    size_t start = 0;

    while ((start = str.find(find, start)) != std::string::npos)
    {
        str.replace(start, find.length(), replace);
        start += replace.length();  // Handles case where 'to' is a substring of 'from'
    }
}
//=============================================================================
bool CerberusUtils::normalize(std::string& str)
{
    bool ret = false;

    CerberusUtils::replaceAll(str, "\r", "\\r");
    CerberusUtils::replaceAll(str, "\n", "\\n\n");

    for (auto&& el : str)
    {
        if ((el < 32 || el > 126) && el != '\n' && el != 0)
        {
            el  = '#';
            ret = true;
        }
    }

    return ret;
}
//=============================================================================
std::string CerberusUtils::truncStr(const std::string& str, SIZE size)
{
    if (size >= str.size())
    {
        return str;
    }

    return str.substr(0, size);
}
//=============================================================================
std::string CerberusUtils::substrUntil(const std::string& str, const std::string& token)
{
    auto found = str.find(token);

    if (found == std::string::npos) return str;

    return str.substr(0, found);
}
//=============================================================================
std::string CerberusUtils::substrFrom(const std::string& str, const std::string& token)
{
    auto found = str.find(token);

    if (found == std::string::npos) return "";

    return str.substr(found + token.size());
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
slot_ptr CerberusUtils::newSlot(SlotType type, const std::string& name)
{
    slot_ptr ret;

    switch (type)
    {
        case ST_BYTE:
            ret = ByteSlot::create();
            break;
        case ST_INT32:
            ret = Int32Slot::create();
            break;
        case ST_INT64:
            ret = Int64Slot::create();
            break;
        case ST_FLOAT:
            ret = FloatSlot::create();
            break;
        case ST_DOUBLE:
            ret = DoubleSlot::create();
            break;
        case ST_BOOL:
            ret = BoolSlot::create();
            break;
        case ST_VOIDP:
            ret = VoidPSlot::create();
            break;
        case ST_STRING:
            ret = StringSlot::create();
            break;
        case ST_BYTEBUFFER:
            ret = BufferSlot::create();
            break;
        case ST_DICTIONARY:
            ret = DictionarySlot::create();
            break;
        case ST_JSON:
            ret = JsonSlot::create();
            break;
        case ST_UINT64:
            ret = UInt64Slot::create();
            break;
        case ST_HOST:
            ret = HostSlot::create();
            break;
        case ST_TASK:
            ret = TaskSlot::create();
            break;
        case ST_RESULT:
            ret = ResultSlot::create();
            break;
        default:
            throw cIllegalArgExc("Unknown slot type");
    }

    ret->name(name);
    return ret;
}
//=============================================================================
MessageTemplate CerberusUtils::standardTemplate(HASH32 id)
{
    MessageTemplate tmplt;

    switch (id)
    {
        case CERBERUS_MESSAGE_LOG_ID:
            tmplt.addSlotType(ST_STRING);
            break;

        case CERBERUS_MESSAGE_TERM_ID:
            // nothing to add
            break;

        case CERBERUS_MESSAGE_TASK_ID:
            tmplt.addSlotType(ST_UINT64, "client");
            tmplt.addSlotType(ST_TASK, "task");
            break;

        case CERBERUS_MESSAGE_TASKEND_ID:
            tmplt.addSlotType(ST_RESULT, "result");
            tmplt.addSlotType(ST_VOIDP, "player");
            break;

            // add here more message specializations..

        default:
            throw cImplMissExc("Requested standard message is not defined");
    }

    return tmplt;
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
StringOpRes CerberusUtils::completePath(const std::string& path)
{
    auto comp = ::realpath(path.c_str(), NULL);

    if (comp == NULL) return OR_Failure;

    std::string ret(comp);
    free(comp);
    return ret;
}
//=============================================================================
SQLDataType CerberusUtils::toSQLDataType(const std::string& type)
{
    if (type.compare("bigint") == 0)
        return SQLDataType::SDT_BigInt;

    else if (type.compare("integer") == 0)
        return SQLDataType::SDT_Int;

    else if (type.compare("smallint") == 0)
        return SQLDataType::SDT_SmallInt;

    else if (type.compare("real") == 0)
        return SQLDataType::SDT_Real;

    else if (type.compare("double precision") == 0)
        return SQLDataType::SDT_Double;

    else if (type.compare("boolean") == 0)
        return SQLDataType::SDT_Boolean;

    else if (type.compare("money") == 0)
        return SQLDataType::SDT_Money;

    else if (CerberusUtils::contains(type, "character"))
    {
        if (CerberusUtils::contains(type, "varying"))
            return SQLDataType::SDT_VarChar;

        else
            return SQLDataType::SDT_Char;
    }
    else if (CerberusUtils::contains(type, "bit"))
    {
        if (CerberusUtils::contains(type, "varying"))
            return SQLDataType::SDT_VarBit;

        else
            return SQLDataType::SDT_Bit;
    }

    return SQLDataType::SDT_Undefined;
}
//=============================================================================
std::string CerberusUtils::fromSQLDataType(SQLDataType type)
{
    switch (type)
    {
        case SQLDataType::SDT_Undefined:
            return "";

        case SQLDataType::SDT_Int:
            return "integer";

        case SQLDataType::SDT_SmallInt:
            return "smallint";

        case SQLDataType::SDT_BigInt:
            return "bigint";

        case SQLDataType::SDT_Real:
            return "real";

        case SQLDataType::SDT_Double:
            return "double precision";

        case SQLDataType::SDT_Boolean:
            return "boolean";

        case SQLDataType::SDT_Bit:
            return "bit";

        case SQLDataType::SDT_VarBit:
            return "bit varying";

        case SQLDataType::SDT_Char:
            return "char";

        case SQLDataType::SDT_VarChar:
            return "char varying";

        case SQLDataType::SDT_Money:
            return "money";
    }

    return "";
}
//=============================================================================
