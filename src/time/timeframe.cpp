#include "timeframe.h"

#define US_IN_MS (uint64_t)1000u
#define US_IN_S (uint64_t)1000000u
#define US_IN_MIN (uint64_t)60000000u
#define US_IN_H (uint64_t)3600000000u
#define US_IN_D (uint64_t)86400000000u
#define US_IN_MON (uint64_t)2592000000000u
#define US_IN_Y (uint64_t)31536000000000u

using namespace crb;

//=============================================================================
void TimeFrame::_add(uint64_t val)
{
    if (isNegative())
    {
        if (val >= m_us)
        {
            m_negative = false;
            m_us       = val - m_us;
        }
        else
        {
            m_us -= val;
        }
    }
    else
    {
        m_us += val;
    }
}
//=============================================================================
void TimeFrame::_subtract(uint64_t val)
{
    if (isNegative())
    {
        m_us -= val;
    }
    else
    {
        if (val > m_us)
        {
            m_negative = true;
            m_us       = val - m_us;
        }
        else
        {
            m_us -= val;
        }
    }
}
//=============================================================================
TimeFrame::TimeFrame()
    : m_us(0),
      m_negative(false)
{
    // noop
}
//=============================================================================
TimeFrame::TimeFrame(uint64_t count, Unit unit)
    : m_us(0),
      m_negative(false)
{
    switch (unit)
    {
        case U_MicroSecond:
            m_us = count;
            break;

        case U_MilliSecond:
            m_us = count * US_IN_MS;
            break;

        case U_Second:
            m_us = count * US_IN_S;
            break;

        case U_Minute:
            m_us = count * US_IN_MIN;
            break;

        case U_Hour:
            m_us = count * US_IN_H;
            break;

        case U_Day:
            m_us = count * US_IN_D;
            break;

        case U_Month:
            m_us = count * US_IN_MON;
            break;

        case U_Year:
            m_us = count * US_IN_Y;
            break;
    }
}
//=============================================================================
bool TimeFrame::isNull() const { return (m_us == 0); }
//=============================================================================
bool TimeFrame::isNegative() const { return m_negative; }
//=============================================================================
void TimeFrame::setNegative(bool neg) { m_negative = neg; }
//=============================================================================
uint64_t TimeFrame::microseconds() const { return m_us % US_IN_MS; }
//=============================================================================
uint64_t TimeFrame::milliseconds() const { return (m_us % US_IN_S) / US_IN_MS; }
//=============================================================================
uint64_t TimeFrame::seconds() const { return (m_us % US_IN_MIN) / US_IN_S; }
//=============================================================================
uint64_t TimeFrame::minutes() const { return (m_us % US_IN_H) / US_IN_MIN; }
//=============================================================================
uint64_t TimeFrame::hours() const { return (m_us % US_IN_D) / US_IN_H; }
//=============================================================================
uint64_t TimeFrame::days() const { return (m_us % US_IN_MON) / US_IN_D; }
//=============================================================================
uint64_t TimeFrame::months() const { return (m_us % US_IN_Y) / US_IN_MON; }
//=============================================================================
uint64_t TimeFrame::years() const { return m_us / US_IN_Y; }
//=============================================================================
uint64_t TimeFrame::toMicroseconds() const { return m_us; }
//=============================================================================
uint64_t TimeFrame::toMilliseconds() const { return m_us / US_IN_MS; }
//=============================================================================
uint64_t TimeFrame::toSeconds() const { return m_us / US_IN_S; }
//=============================================================================
uint64_t TimeFrame::toMinutes() const { return m_us / US_IN_MIN; }
//=============================================================================
uint64_t TimeFrame::toHours() const { return m_us / US_IN_H; }
//=============================================================================
uint64_t TimeFrame::toDays() const { return m_us / US_IN_D; }
//=============================================================================
uint64_t TimeFrame::toMonths() const { return m_us / US_IN_MON; }
//=============================================================================
SplittedTime TimeFrame::splittedTime() const
{
    SplittedTime ret{};
    ret.seconds = seconds();

    uint64_t time   = m_us % US_IN_S;
    ret.nanoseconds = time * 1000u;

    return ret;
}
//=============================================================================
TimeFrame &TimeFrame::addMicroseconds(int64_t time)
{
    if (time < 0)
    {
        uint64_t val = -time;
        subtract(val);
    }
    else
        add(time);

    return *this;
}
//=============================================================================
TimeFrame &TimeFrame::setMicroseconds(uint64_t time)
{
    m_us = time;
    return *this;
}
//=============================================================================
TimeFrame &TimeFrame::add(const TimeFrame &other)
{
    if (other.isNegative())
        _subtract(other.m_us);
    else
        _add(other.m_us);

    return *this;
}
//=============================================================================
TimeFrame &TimeFrame::subtract(const TimeFrame &other)
{
    if (other.isNegative())
        _add(other.m_us);
    else
        _subtract(other.m_us);

    return *this;
}
//=============================================================================
TimeFrame TimeFrame::operator+(const TimeFrame &other)
{
    TimeFrame ret(*this);
    ret.add(other);
    return ret;
}
//=============================================================================
void TimeFrame::operator+=(const TimeFrame &other) { add(other); }
//=============================================================================
TimeFrame TimeFrame::operator-(const TimeFrame &other)
{
    TimeFrame ret(*this);
    ret.subtract(other);
    return ret;
}
//=============================================================================
void TimeFrame::operator-=(const TimeFrame &other) { subtract(other); }
//=============================================================================
bool TimeFrame::operator<(const TimeFrame &other) const
{
    if (isNegative())
    {
        if (other.isNegative())
            return m_us > other.m_us;
        else
            return true;
    }
    else
    {
        if (other.isNegative())
            return false;
        else
            return m_us < other.m_us;
    }
}
//=============================================================================
bool TimeFrame::operator>(const TimeFrame &other) const
{
    if (isNegative())
    {
        if (other.isNegative())
            return m_us < other.m_us;
        else
            return false;
    }
    else
    {
        if (other.isNegative())
            return true;
        else
            return m_us > other.m_us;
    }
}
//=============================================================================
bool TimeFrame::operator<=(const TimeFrame &other) const
{
    if (isNegative() == other.isNegative() && m_us == other.m_us) return true;

    if (isNegative())
    {
        if (other.isNegative())
            return m_us > other.m_us;
        else
            return true;
    }
    else
    {
        if (other.isNegative())
            return false;
        else
            return m_us < other.m_us;
    }
}
//=============================================================================
bool TimeFrame::operator>=(const TimeFrame &other) const
{
    if (isNegative() == other.isNegative() && m_us == other.m_us) return true;

    if (isNegative())
    {
        if (other.isNegative())
            return m_us < other.m_us;
        else
            return false;
    }
    else
    {
        if (other.isNegative())
            return true;
        else
            return m_us > other.m_us;
    }
}
//=============================================================================
bool TimeFrame::operator==(const TimeFrame &other) const
{
    if (isNegative() == other.isNegative() && m_us == other.m_us) return true;

    return false;
}
//=============================================================================
bool TimeFrame::operator!=(const TimeFrame &other) const { return !((*this) == other); }
//=============================================================================
