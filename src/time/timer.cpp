#include "timer.h"

#include "src/cerberus.h"

using namespace cerberus::time;

//=============================================================================
void Timer::defaultTimeoutCallback()
{
    // noop
}
//=============================================================================
Timer::Timer()
    : m_running(false),
      m_periodic(false),
      m_time(),
      m_callback(defaultTimeoutCallback)
{
}
//=============================================================================
Timer::Timer(const TimeFrame &time, bool periodic)
    : m_running(false),
      m_periodic(periodic),
      m_time(time),
      m_callback(defaultTimeoutCallback)
{
}
//=============================================================================
Timer::~Timer() { stop(); }
//=============================================================================
void Timer::setTime(const TimeFrame &time) { m_time = time; }
//=============================================================================
void Timer::start()
{
    auto &i = cerberus::Cerberus::instance();

    if (m_periodic)
        i.m_eventScheduler.startTimer(m_running, m_time, m_callback);
    else
        i.m_eventScheduler.startTimer(m_running, time::DateTime::current().add(m_time), m_callback);
}
//=============================================================================
void Timer::stop()
{
    auto &i = cerberus::Cerberus::instance();
    i.m_eventScheduler.stopTimer(m_running);
}
//=============================================================================
void Timer::reset()
{
    stop();
    start();
}
//=============================================================================
bool Timer::isRunning() { return m_running; }
//=============================================================================
void Timer::provideTimeoutCallback(timerCallback callback) { m_callback = callback; }
//=============================================================================
