#include "timer.h"

#include "src/cerberus.h"

using namespace cerberus;

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
    if (m_periodic)
        Cerberus::startTimer(m_running, m_time, m_callback);
    else
        Cerberus::startTimer(m_running, DateTime::current().add(m_time), m_callback);
}
//=============================================================================
void Timer::stop() { Cerberus::stopTimer(m_running); }
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
