#include "timer.h"

#include "src/cerberus.h"

using namespace cerberus;

//=============================================================================
void Timer::defaultTimeoutCallback(void *ctx)
{
    (void)ctx;
    // noop
}
//=============================================================================
Timer::Timer()
    : m_running(false),
      m_type(TT_OneShot),
      m_callback(defaultTimeoutCallback),
      m_ctx(nullptr)
{
}
//=============================================================================
Timer::Timer(const TimeFrame &time, TimerType type, const DateTime &delay)
    : m_running(false),
      m_type(type),
      m_time(time),
      m_delay(delay),
      m_callback(defaultTimeoutCallback),
      m_ctx(nullptr)
{
}
//=============================================================================
Timer::~Timer() { stop(); }
//=============================================================================
void Timer::setTime(const TimeFrame &time, const DateTime &delay)
{
    m_time  = time;
    m_delay = delay;
}
//=============================================================================
void Timer::setTime(const DateTime &delay) { m_delay = delay; }
//=============================================================================
void Timer::start()
{
    TimerData td = {};

    td.bit      = &m_running;
    td.callback = m_callback;
    td.ctx      = m_ctx;

    switch (m_type)
    {
        case TT_OneShot:
            td.delay = DateTime::current().add(m_time);
            break;

        case TT_Periodic:
            td.time = m_time;
            break;

        case TT_Delayed:
            td.time  = m_time;
            td.delay = m_delay;
            break;

        case TT_Alarm:
            td.delay = m_delay;
            break;
    };

    Cerberus::startTimer(td);
}
//=============================================================================
void Timer::stop()
{
    if (!m_running.load(std::memory_order_relaxed)) return;

    Cerberus::stopTimer(m_running);
}
//=============================================================================
bool Timer::isRunning() { return m_running.load(std::memory_order_relaxed); }
//=============================================================================
void Timer::provideTimeoutCallback(timerCallback callback, void *ctx)
{
    m_callback = callback;
    m_ctx      = ctx;
}
//=============================================================================
