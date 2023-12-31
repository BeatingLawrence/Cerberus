#include "cerberusutils.h"

#include <algorithm>
#include <cstdarg>
#include <cstdlib>
#include <cstring>

#include "src/cerberus.h"

using namespace cerberus::core;

std::regex CerberusUtils::isNumberRegex("\\-?[0-9]+(?:\\.[0-9]+)?", std::regex_constants::ECMAScript | std::regex_constants::optimize | std::regex_constants::icase);

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
std::string CerberusUtils::removeBlank_copy(const std::string& str) { return removeBlankAfter(removeBlankBefore(str)); }
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
            auto s1 = str1;
            auto s2 = str2;
            toLower(s1);
            toLower(s2);

            if (s1.compare(s2) == 0)
            {
                return true;
            }
            break;
    }

    return false;
}
//=============================================================================
cerberus::OperationResult CerberusUtils::environmentVariable(const std::string& variableName)
{
    char* val = getenv(variableName.c_str());

    if (val == nullptr)
    {
        return OR_NotFound;
    }

    return std::string(val);
}
//=============================================================================
long long int CerberusUtils::stringToInt(const std::string& str, Radix r)
{
    long long int ret = 0;

    try
    {
        ret = std::stoll(str, nullptr, r == Radix::Binary ? 2 : r == Radix::Decimal ? 10 : r == Radix::Hexadecimal ? 16 : 0);
    }
    catch (...)
    {
    }

    return ret;
}
//=============================================================================
long double CerberusUtils::stringToDouble(const std::string& str)
{
    long double ret = 0.0f;

    try
    {
        ret = std::stold(str, nullptr);
    }
    catch (...)
    {
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
bool CerberusUtils::isBool(const std::string& str, WordMatch match) { return (CerberusUtils::areEqual(str, "true", match) || CerberusUtils::areEqual(str, "false", match)); }
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

    core::CerberusUtils::replaceAll(str, "\r", "\\r");
    core::CerberusUtils::replaceAll(str, "\n", "\\n\n");

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
cerberus::DoubleString CerberusUtils::split(const std::string& str, const std::string& token) { return {substrUntil(str, token), substrFrom(str, token)}; }
//=============================================================================
cerberus::OperationResult CerberusUtils::cleanNumber(std::string& str)
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
