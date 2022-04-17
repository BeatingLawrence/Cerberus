#include "cerberus.h"
#include <cstring>
#include <cstdarg>

using namespace cerberus;

//=============================================================================
Cerberus::Cerberus()
{
}
//=============================================================================
std::string Cerberus::strPrint(const char* format, ...)
{
    std::string ret;

    if(format != nullptr)
    {
        if(strlen(format) != 0)
        {
            va_list testList;
            va_list list;
            va_start(testList, format);
            va_copy(list, testList);
            char garbage;
            int required = vsnprintf(&garbage, 0, format, testList);

            if(required > 0)
            {
                ret.resize(required);
                vsnprintf(&ret[0], required + 1, format, list);
            }

            va_end(testList);
            va_end(list);
        }
    }

    return ret;
}
//=============================================================================
