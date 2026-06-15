#ifndef CERBERUS_CORE_CERBERUSREGEX_H
#define CERBERUS_CORE_CERBERUSREGEX_H

#include <cstdint>
#include <memory>
#include <string>

#include "../Cerberus_global.h"

namespace crb
{
    class CerberusRegex
    {
       public:
        enum Flag : uint32_t
        {
            ECMAScript = 1u << 0,
            Perl       = 1u << 1,
            ICase      = 1u << 2,
            Optimize   = 1u << 3
        };

        CERBERUS_EXPORT CerberusRegex();
        CERBERUS_EXPORT CerberusRegex(const std::string& pattern, uint32_t flags = ECMAScript);
        CERBERUS_EXPORT CerberusRegex(const CerberusRegex& other);
        CERBERUS_EXPORT CerberusRegex(CerberusRegex&& other) noexcept;
        CERBERUS_EXPORT ~CerberusRegex();

        CERBERUS_EXPORT CerberusRegex& operator=(const CerberusRegex& other);
        CERBERUS_EXPORT CerberusRegex& operator=(CerberusRegex&& other) noexcept;

        CERBERUS_EXPORT void assign(const std::string& pattern, uint32_t flags = ECMAScript);
        CERBERUS_EXPORT bool match(const std::string& str) const;
        CERBERUS_EXPORT bool search(const std::string& str) const;

       private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };
}  // namespace crb

#endif  // CERBERUS_CORE_CERBERUSREGEX_H
