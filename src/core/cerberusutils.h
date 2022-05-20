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
            public:
                CerberusUtils() = delete;
                CerberusUtils(const CerberusUtils& other) = delete;

                //Prints formatted content on a std::string which is returned
                static std::string strPrint(const char* format, ...);
        };
    }
}

#endif // CERBERUS_CORE_CERBERUSUTILS_H
