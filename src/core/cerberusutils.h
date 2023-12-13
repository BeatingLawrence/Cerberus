#ifndef CERBERUS_CORE_CERBERUSUTILS_H
#define CERBERUS_CORE_CERBERUSUTILS_H

/*  This class provides a set of tools for general purpose.
 *
 *  Many components of this class are used in the Cerberus Framework, but
 *  they have been made available here to be used by developers too.
 *
 *  This class contains static methods only
 */

#include "../Cerberus_global.h"
#include "../types.h"

namespace cerberus
{
    namespace core
    {
        class CERBERUS_EXPORT CerberusUtils
        {
            CerberusUtils()                           = delete;
            CerberusUtils(const CerberusUtils& other) = delete;

           public:
            // Prints formatted content on a std::string which is returned
            static std::string strPrint(std::string format, ...);

            // Returns the lower case version of str
            static std::string toLower(const std::string& str);

            // Returns the upper case version of str
            static std::string toUpper(const std::string& str);

            // Removes spaces before str and returns the result.
            static std::string removeBlankBefore(const std::string& str);

            // Removes spaces after str and returns the result.
            static std::string removeBlankAfter(const std::string& str);

            // Removes spaces before and after str. Spaces between characters will remain
            static std::string removeBlank_copy(const std::string& str);
            static void removeBlank(std::string& str);

            // Checks if str1 contains str2 and returns true if it does
            static bool contains(const std::string& str1, const std::string& str2);

            // Returns true when given strings are equal (case sensitive)
            static bool areEqual(const std::string& str1, const std::string& str2);

            // Retrieves the value of specified environment variable
            static std::string environmentVariable(const std::string& variableName);

            // Converts a given string to an int
            static int stringToInt(const std::string& str, Radix r = Radix::Decimal);

            // Tells if the string contains at least one alphabet character [a-z][A-Z]
            static bool isAlpha(const std::string& str);

            // Check if str1 starts with str2 string
            static bool startsWith(const std::string& str1, const std::string& str2);

            // Check if str1 starts with c char
            static bool startsWith(const std::string& str, char c);

            // Check if str1 ends with str2 string
            static bool endsWith(const std::string& str1, const std::string& str2);

            // Check if str1 ends with c char
            static bool endsWith(const std::string& str, char c);

            // Replace all the occurrences of find in str with replace
            static void replaceAll(std::string& str, const std::string& find, const std::string& replace);

            // Normalize the given string. After the normalization the string will contain pure text
            // and will be printable on a terminal or a text file safely.
            // line-terminating characters are replaced with \n or \r\n
            // other non-textual characters are replaced with #
            static void normalize(std::string& str);

            // Truncate the given string making it long at most size chars.
            // If size is bigger than (or equal to) the str size, the str string is returned
            static std::string truncStr(const std::string& str, SIZE size);
        };
    }  // namespace core
}  // namespace cerberus

#endif  // CERBERUS_CORE_CERBERUSUTILS_H
