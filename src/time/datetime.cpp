#include "datetime.h"

#include <climits>
#include <string.h>

#include <chrono>

#include "src/cerberus.h"
#include "src/core/cerberusutils.h"
#include "src/exception/exception.h"

using namespace crb;

//=============================================================================
static inline tm toTm(const DateTime& dt)
{
    tm t = {};
    t.tm_year = static_cast<int>(dt.years()) - 1900;
    t.tm_mon  = static_cast<int>(dt.months()) - 1;
    t.tm_mday = static_cast<int>(dt.days());
    t.tm_hour = static_cast<int>(dt.hours());
    t.tm_min  = static_cast<int>(dt.minutes());
    t.tm_sec  = static_cast<int>(dt.seconds());
    t.tm_isdst = -1;
    return t;
}

static void addTmSeconds(tm& time, uint64_t seconds)
{
    while (seconds > static_cast<uint64_t>(INT_MAX))
    {
        time.tm_sec += INT_MAX;
        mktime(&time);
        seconds -= static_cast<uint64_t>(INT_MAX);
    }

    time.tm_sec += static_cast<int>(seconds);
}

static void subtractTmSeconds(tm& time, uint64_t seconds)
{
    while (seconds > static_cast<uint64_t>(INT_MAX))
    {
        time.tm_sec -= INT_MAX;
        mktime(&time);
        seconds -= static_cast<uint64_t>(INT_MAX);
    }

    time.tm_sec -= static_cast<int>(seconds);
}
//=============================================================================
time_t DateTime::normalize() const
{
    if (m_offset.isNegative())
    {
        m_offset.setNegative(false);
        subtractTmSeconds(m_time, (m_offset.toMicroseconds() / 1000000u) + 1);
        m_offset.setMicroseconds(1000000u - (m_offset.microseconds() % 1000000u));
    }
    else if (m_offset.toMicroseconds() >= 1000000u)
    {
        addTmSeconds(m_time, m_offset.toMicroseconds() / 1000000u);
        m_offset.setMicroseconds(m_offset.toMicroseconds() % 1000000u);
    }

    return mktime(&m_time);
}
//=============================================================================
DateTime::DateTime()
    : m_time(),
      m_offset(0),
      m_zone()
{
#ifndef WINDOWS_SYSTEM
    m_time.tm_zone = &m_zone[0];
#endif
}
//=============================================================================
DateTime::DateTime(int64_t seconds, int64_t nanoseconds)
    : m_time(),
      m_offset(0),
      m_zone()
{
#ifndef WINDOWS_SYSTEM
    m_time.tm_zone = &m_zone[0];
#endif
    fromTimespec(seconds, nanoseconds);
}
//=============================================================================
bool DateTime::isValid() const
{
    return !m_offset.isNull() || (m_time.tm_sec || m_time.tm_min || m_time.tm_hour || m_time.tm_mday ||
                                  m_time.tm_mon || m_time.tm_year);
}
//=============================================================================
bool DateTime::usingDst() const { return m_time.tm_isdst > 0; }
//=============================================================================
uint32_t DateTime::microseconds() const { return static_cast<uint32_t>(m_offset.microseconds()); }
//=============================================================================
uint32_t DateTime::milliseconds() const { return static_cast<uint32_t>(m_offset.milliseconds()); }
//=============================================================================
uint32_t DateTime::seconds() const { return m_time.tm_sec; }
//=============================================================================
uint32_t DateTime::minutes() const { return m_time.tm_min; }
//=============================================================================
uint32_t DateTime::hours() const { return m_time.tm_hour; }
//=============================================================================
uint32_t DateTime::days() const { return m_time.tm_mday; }
//=============================================================================
uint32_t DateTime::months() const { return m_time.tm_mon + 1; }
//=============================================================================
uint32_t DateTime::years() const { return m_time.tm_year + 1900; }
//=============================================================================
DateTime &DateTime::addMicroseconds(int x)
{
    m_offset.addMicroseconds(x);
    normalize();
    return *this;
}
//=============================================================================
DateTime &DateTime::addMilliseconds(int x)
{
    m_offset.addMicroseconds(x * 1000u);
    normalize();
    return *this;
}
//=============================================================================
DateTime &DateTime::addSeconds(int x)
{
    m_time.tm_sec += x;
    normalize();
    return *this;
}
//=============================================================================
DateTime &DateTime::addMinutes(int x)
{
    m_time.tm_min += x;
    normalize();
    return *this;
}
//=============================================================================
DateTime &DateTime::addHours(int x)
{
    m_time.tm_hour += x;
    normalize();
    return *this;
}
//=============================================================================
DateTime &DateTime::addDays(int x)
{
    m_time.tm_mday += x;
    normalize();
    return *this;
}
//=============================================================================
DateTime &DateTime::addMonths(int x)
{
    m_time.tm_mon += x;
    normalize();
    return *this;
}
//=============================================================================
DateTime &DateTime::addYears(int x)
{
    m_time.tm_year += x;
    normalize();
    return *this;
}
//=============================================================================
DateTime &DateTime::setMicroseconds(uint64_t x)
{
    m_offset.setMicroseconds(x);
    normalize();
    return *this;
}
//=============================================================================
DateTime &DateTime::setMilliseconds(uint64_t x)
{
    m_offset.setMicroseconds(x * 1000u);
    normalize();
    return *this;
}
//=============================================================================
DateTime &DateTime::setSeconds(uint64_t x)
{
    m_time.tm_sec = 0;
    addTmSeconds(m_time, x);
    normalize();
    return *this;
}
//=============================================================================
DateTime &DateTime::setMinutes(uint32_t x)
{
    m_time.tm_min = x;
    normalize();
    return *this;
}
//=============================================================================
DateTime &DateTime::setHours(uint32_t x)
{
    m_time.tm_hour = x;
    normalize();
    return *this;
}
//=============================================================================
DateTime &DateTime::setDays(uint32_t x)
{
    m_time.tm_mday = x;
    normalize();
    return *this;
}
//=============================================================================
DateTime &DateTime::setMonths(uint32_t x)
{
    m_time.tm_mon = x - 1;
    normalize();
    return *this;
}
//=============================================================================
DateTime &DateTime::setYears(uint32_t x)
{
    m_time.tm_year = x - 1900;
    normalize();
    return *this;
}
//=============================================================================
DateTime &DateTime::add(const TimeFrame &time)
{
    m_offset += time;
    normalize();
    return *this;
}
//=============================================================================
DateTime &DateTime::subtract(const TimeFrame &time)
{
    m_offset -= time;
    normalize();
    return *this;
}
//=============================================================================
DateTime &DateTime::operator+(const TimeFrame &other) { return add(other); }
//=============================================================================
DateTime &DateTime::operator-(const TimeFrame &other) { return subtract(other); }
//=============================================================================
bool DateTime::operator<(const DateTime &other) const
{
    auto thisTime  = normalize();
    auto otherTime = other.normalize();

    if (thisTime == otherTime)
    {
        return m_offset < other.m_offset;
    }

    return thisTime < otherTime;
}
//=============================================================================
bool DateTime::operator<=(const DateTime &other) const
{
    auto thisTime  = normalize();
    auto otherTime = other.normalize();

    if (thisTime == otherTime)
    {
        return m_offset <= other.m_offset;
    }

    return thisTime < otherTime;
}
//=============================================================================
bool DateTime::operator>(const DateTime &other) const
{
    auto thisTime  = normalize();
    auto otherTime = other.normalize();

    if (thisTime == otherTime)
    {
        return m_offset > other.m_offset;
    }

    return thisTime > otherTime;
}
//=============================================================================
bool DateTime::operator>=(const DateTime &other) const
{
    auto thisTime  = normalize();
    auto otherTime = other.normalize();

    if (thisTime == otherTime)
    {
        return m_offset >= other.m_offset;
    }

    return thisTime > otherTime;
}
//=============================================================================
bool DateTime::isOlder(const DateTime &other) const { return (*this) < other; }
//=============================================================================
bool DateTime::isNewer(const DateTime &other) const { return (*this) > other; }
//=============================================================================
std::string DateTime::toString() const
{
    return CerberusUtils::strPrint("%.4u/%.2u/%.2u %.2u:%.2u:%.2u", years(), months(), days(), hours(),
                                   minutes(), seconds());
}
//=============================================================================
std::string DateTime::toTimeStampString() const
{
    return CerberusUtils::strPrint("%.4u.%.2u.%.2u-%.2u:%.2u:%.2u.%.3u", years(), months(), days(), hours(),
                                   minutes(), seconds(), milliseconds());
}
//=============================================================================
uint64_t DateTime::toEpochMilliseconds() const
{
    tm t = toTm(*this);
    time_t seconds = mktime(&t);
    if (seconds < 0) seconds = 0;
    return static_cast<uint64_t>(seconds) * 1000u + static_cast<uint64_t>(milliseconds());
}
//=============================================================================
DateTime &DateTime::fromTimespec(int64_t seconds, int64_t nanoseconds)
{
    m_offset.setMicroseconds(static_cast<uint64_t>(nanoseconds / 1000));

    time_t t = static_cast<time_t>(seconds);

#ifdef WINDOWS_SYSTEM
    if (localtime_s(&m_time, &t) != 0)
#else
    if (!localtime_r(&t, &m_time))
#endif
    {
        throw cSystemExc("gmtime failure: %u", errno);
    }

    // normalize();
    return *this;
}
//=============================================================================
DateTime DateTime::current()
{
#ifdef WINDOWS_SYSTEM
    const auto now = std::chrono::system_clock::now();
    const auto duration = now.time_since_epoch();
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    const auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration - seconds);
    return DateTime(seconds.count(), nanoseconds.count());
#else
    timespec ts = {};

    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
    {
        logDebug("clock_gettime error: %s", strerror(errno));
    }

    return DateTime(ts.tv_sec, ts.tv_nsec);
#endif
}
//=============================================================================
