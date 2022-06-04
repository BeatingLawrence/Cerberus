#ifndef CERBERUS_CORE_CERBERUSUTILS_H
#define CERBERUS_CORE_CERBERUSUTILS_H

/*  This class provides a set of tools for general purpose.
 *
 *  Many components of this class are used in the Cerberus Framework, but
 *  they have been made available here to be used my developers.
 *
 *  This class contains static methods only
 */

#include <string>
#include "../Cerberus_global.h"

namespace cerberus
{
    namespace core
    {
        class CERBERUS_EXPORT CerberusUtils
        {
                CerberusUtils() = delete;
                CerberusUtils(const CerberusUtils& other) = delete;

            public:
                //Prints formatted content on a std::string which is returned
                static std::string strPrint(const char* format, ...);

                //Returns the lower case version of str
                static std::string toLower(const std::string& str);

                //Returns the upper case version of str
                static std::string toUpper(const std::string& str);

                //Removes spaces before str and returns the result.
                static std::string removeBlankBefore(const std::string& str);

                //Removes spaces after str and returns the result.
                static std::string removeBlankAfter(const std::string& str);

                //Removes spaces before and after str. Spaces between characters will remain
                static std::string removeBlank_copy(const std::string& str);
                static void removeBlank(std::string& str);

                //Retrieves the value of specified environment variable
                static std::string environmentVariable(const std::string& variableName);
        };
    }
}

#endif // CERBERUS_CORE_CERBERUSUTILS_H
