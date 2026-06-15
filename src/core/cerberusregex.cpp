#include "cerberusregex.h"

#include <boost/regex.hpp>

using namespace crb;

struct CerberusRegex::Impl
{
    boost::regex regex;

    Impl()
        : regex() {}

    Impl(const std::string& pattern, uint32_t flags)
        : regex(pattern, toBoostFlags(flags)) {}

    static boost::regex::flag_type toBoostFlags(uint32_t flags)
    {
        boost::regex::flag_type out = boost::regex::ECMAScript;

        if (flags & CerberusRegex::Perl)
            out = boost::regex::perl;
        else if (flags & CerberusRegex::ECMAScript)
            out = boost::regex::ECMAScript;

        if (flags & CerberusRegex::ICase) out |= boost::regex::icase;
        if (flags & CerberusRegex::Optimize) out |= boost::regex::optimize;

        return out;
    }
};

//=============================================================================
CerberusRegex::CerberusRegex()
    : m_impl(new Impl()) {}
//=============================================================================
CerberusRegex::CerberusRegex(const std::string& pattern, uint32_t flags)
    : m_impl(new Impl(pattern, flags)) {}
//=============================================================================
CerberusRegex::CerberusRegex(const CerberusRegex& other)
    : m_impl(new Impl())
{
    m_impl->regex = other.m_impl->regex;
}
//=============================================================================
CerberusRegex::CerberusRegex(CerberusRegex&& other) noexcept = default;
//=============================================================================
CerberusRegex::~CerberusRegex() = default;
//=============================================================================
CerberusRegex& CerberusRegex::operator=(const CerberusRegex& other)
{
    if (this == &other) return *this;
    m_impl->regex = other.m_impl->regex;
    return *this;
}
//=============================================================================
CerberusRegex& CerberusRegex::operator=(CerberusRegex&& other) noexcept = default;
//=============================================================================
void CerberusRegex::assign(const std::string& pattern, uint32_t flags)
{
    m_impl->regex.assign(pattern, Impl::toBoostFlags(flags));
}
//=============================================================================
bool CerberusRegex::match(const std::string& str) const { return boost::regex_match(str, m_impl->regex); }
//=============================================================================
bool CerberusRegex::search(const std::string& str) const { return boost::regex_search(str, m_impl->regex); }
//=============================================================================
