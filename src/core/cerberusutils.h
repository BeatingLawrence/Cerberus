#ifndef CERBERUS_CORE_CERBERUSUTILS_H
#define CERBERUS_CORE_CERBERUSUTILS_H

/*  This class provides a set of tools for general purpose.
 *
 *  Many components of this class are used in the Cerberus Framework, but
 *  they have been made available here to be used by developers too.
 *
 *  This class contains static methods only
 */

#include <regex>

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

            static std::regex isNumberRegex;

           public:
            // Print formatted content on a std::string which is returned
            static std::string strPrint(std::string format, ...);

            // Return the lower case version of str
            static std::string toLower(const std::string& str);

            // Return the upper case version of str
            static std::string toUpper(const std::string& str);

            // Remove spaces or TABs before str and return the result.
            static std::string removeBlankBefore(const std::string& str);

            // Remove spaces or TABs after str and return the result.
            static std::string removeBlankAfter(const std::string& str);

            // Remove spaces or TABs before and after str. Spaces between characters will remain
            static std::string removeBlank_copy(const std::string& str);
            static void removeBlank(std::string& str);

            // Check if str1 contains str2 and return true if it does
            static bool contains(const std::string& str1, const std::string& str2);

            // Check if str1 contains c and return true if it does
            static bool contains(const std::string& str1, char c);

            // Return true when given strings are equal (default = case sensitive)
            static bool areEqual(const std::string& str1, const std::string& str2, WordMatch match = WM_CaseSensitive);

            // Retrieve the value of specified environment variable
            static OperationResult environmentVariable(const std::string& variableName);

            // Convert a given string to an int (64 bit)
            static OperationResult stringToInt(const std::string& str, Radix r = Radix::Decimal);

            // Convert a given string to a long double
            static OperationResult stringToDouble(const std::string& str);

            // Tell if the string contains at least one alphabet character [a-z][A-Z]
            static bool isAlpha(const std::string& str);

            // Tell if the string is a number (floating point or integer).
            // The string must contain the number only for this method to return true
            static bool isNumber(const std::string& str);

            // Tell if the string is a boolean.
            // The string must contain true or false
            static bool isBool(const std::string& str, WordMatch match = WM_CaseInsensitive);

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
            // If some non-textual character is found, this method returns true
            static bool normalize(std::string& str);

            // Truncate the given string making it long at most size chars.
            // If size is bigger than (or equal to) the str size, the str string is returned
            static std::string truncStr(const std::string& str, SIZE size);

            // Return a substring of str that starts at pos 0 and ends at token pos (token excluded)
            // If token was not found, the returned string will be equal to the str parameter.
            // The token is searched from the start of the string
            static std::string substrUntil(const std::string& str, const std::string& token);

            // Return a substring of str that starts at token pos (token excluded) and ends at str end.
            // If token was not found, the returned string will be empty.
            // The token is searched from the start of the string
            static std::string substrFrom(const std::string& str, const std::string& token);

            // Splits the given str in two pieces at the given token (that is excluded from both).
            // If token was not found, all str will be placed in the left string returned, and the
            // right string will be empty.
            // The token is searched from the start of the string
            static DoubleString split(const std::string& str, const std::string& token);

            // Clean the number contained inside str (floating or decimal) removing
            // trailing or leading zeros (without altering the value of course).
            // The method also removes the dot if the decimal part is null.
            // This method fails if the supplied string is not a number
            static OperationResult cleanNumber(std::string& str);

            // Construct a new Slot
            static cerberus_slot newSlot(SlotType type);
        };
    }  // namespace core
}  // namespace cerberus

#endif  // CERBERUS_CORE_CERBERUSUTILS_H
