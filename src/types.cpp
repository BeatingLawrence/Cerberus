#include "./types.h"

#include "./core/cerberusutils.h"

//=============================================================================
cerberus::Host::Host() : octet_networkOrder(0), port(0), resolved(false) {}
//=============================================================================
cerberus::Host::Host(const std::string &str) : octet_networkOrder(0), port(0), resolved(false)
{
    if (core::CerberusUtils::isAlpha(str))
    {
        hostname = str;
    }
    else
    {
        fromString(str);
    }
}
//=============================================================================
bool cerberus::Host::fromString(const std::string &str)
{
    auto col = str.find_last_of(':');
    int portint = 0;

    if (col != std::string::npos && col != str.size() - 1)
    {
        // address with port
        std::string portstr = str.substr(col + 1);
        portint = atoi(portstr.c_str());

        if (portint < 0 || portint > 65535)
        {
            return false;
        }
    }

    int32_t oct[4];
    std::string::size_type dots[3];
    std::string ip = str.substr(0, col);
    std::string sub;
    //
    dots[0] = ip.find_first_of('.');
    sub = ip.substr(0, dots[0]);
    oct[0] = atoi(sub.c_str());

    if (oct[0] < 0 || oct[0] > 255 || sub.empty())
    {
        return false;
    }

    dots[1] = ip.find_first_of('.', dots[0] + 1);
    sub = ip.substr(dots[0] + 1, dots[1] - (dots[0] + 1));
    oct[1] = atoi(sub.c_str());

    if (oct[1] < 0 || oct[1] > 255 || sub.empty())
    {
        return false;
    }

    dots[2] = ip.find_first_of('.', dots[1] + 1);
    sub = ip.substr(dots[1] + 1, dots[2] - (dots[1] + 1));
    oct[2] = atoi(sub.c_str());

    if (oct[2] < 0 || oct[2] > 255 || sub.empty())
    {
        return false;
    }

    sub = ip.substr(dots[2] + 1);
    oct[3] = atoi(sub.c_str());

    if (oct[3] < 0 || oct[3] > 255 || sub.empty())
    {
        return false;
    }

    octect[0] = oct[0];
    octect[1] = oct[1];
    octect[2] = oct[2];
    octect[3] = oct[3];
    port = portint;
    return true;
}
//=============================================================================
std::string cerberus::Host::toString() { return cerberus::core::CerberusUtils::strPrint("%u.%u.%u.%u:%u", octect[0], octect[1], octect[2], octect[3], port); }
//=============================================================================
bool cerberus::Host::isValid() { return !(octet_networkOrder == 0 && port == 0 && hostname.empty()); }
//=============================================================================
