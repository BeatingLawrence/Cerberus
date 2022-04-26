#include "time.h"

using namespace cerberus::time;

//=============================================================================
Time::Time() :
    m_period_uS(0)
{
    // noop
}
//=============================================================================
Time::Time(uint64_t count, Unit unit)
{
    switch(unit)
    {
        case U_MicroSecond:
            m_period_uS = count;
            break;

        case U_MilliSecond:
            m_period_uS = count * 1000;
            break;

        case U_Second:
            m_period_uS = count * 1000 * 1000;
            break;

        case U_Minute:
            m_period_uS = count * 1000 * 1000 * 60;
            break;

        case U_Hour:
            m_period_uS = count * 1000 * 1000 * 60 * 60;
            break;
    }
}
//=============================================================================
bool Time::isValid() const
{
    return (m_period_uS != 0);
}
//=============================================================================
uint64_t Time::getMicroseconds() const
{
    return m_period_uS;
}
//=============================================================================
uint64_t Time::getMilliseconds() const
{
    return m_period_uS / 1000u;
}
//=============================================================================
uint64_t Time::getSeconds() const
{
    return m_period_uS / (uint64_t)(1000u * 1000u);
}
//=============================================================================
uint64_t Time::getMinutes() const
{
    return m_period_uS / (uint64_t)(1000u * 1000u * 60u);
}
//=============================================================================
uint64_t Time::getHours() const
{
    return m_period_uS / (uint64_t)(1000u * 1000u * 60u * 60u);
}
//=============================================================================
