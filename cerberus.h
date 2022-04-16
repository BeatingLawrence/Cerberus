#ifndef CERBERUS_H
#define CERBERUS_H

#include "Cerberus_global.h"
#include <string>

namespace cerberus
{
    class CERBERUS_EXPORT Cerberus
    {
        public:
            static std::string print(const char* format, ...);

            Cerberus();
    };
}
#endif // CERBERUS_H
