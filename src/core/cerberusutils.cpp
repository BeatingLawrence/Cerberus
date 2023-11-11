#include "cerberusutils.h"

#include <algorithm>
#include <cstdarg>
#include <cstdlib>
#include <cstring>

using namespace cerberus::core;

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
    size_t start = str.find_first_not_of(' ');
    return (start == std::string::npos) ? "" : str.substr(start);
}
//=============================================================================
std::string CerberusUtils::removeBlankAfter(const std::string& str)
{
    size_t end = str.find_last_not_of(' ');
    return (end == std::string::npos) ? "" : str.substr(0, end + 1);
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
    if (str1.find(str2) == std::string::npos)
    {
        return false;
    }

    return true;
}
//=============================================================================
bool CerberusUtils::areEqual(const std::string& str1, const std::string& str2)
{
    if (str1.compare(str2) == 0)
    {
        return true;
    }

    return false;
}
//=============================================================================
std::string CerberusUtils::environmentVariable(const std::string& variableName)
{
    char* val = getenv(variableName.c_str());

    if (val == nullptr)
    {
        return std::string();
    }

    return std::string(val);
}
//=============================================================================
int CerberusUtils::stringToInt(const std::string& str, Radix r)
{
    int ret = 0;

    try
    {
        ret = std::stoi(str, nullptr, r == Radix::Binary ? 2 : r == Radix::Decimal ? 10 : r == Radix::Hexadecimal ? 16 : 0);
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
void CerberusUtils::normalize(std::string& str)
{
    core::CerberusUtils::replaceAll(str, "\r", "\\r");
    core::CerberusUtils::replaceAll(str, "\n", "\\n\n");

    for (auto&& el : str)
    {
        if ((el < 32 || el > 126) && el != '\n')
        {
            el = '#';
        }
    }
}
//=============================================================================
std::string CerberusUtils::truncStr(const std::string& str, SIZE size)
{
    if (str.size() <= size)
    {
        return str;
    }

    return str.substr(0, size);
}
//=============================================================================
