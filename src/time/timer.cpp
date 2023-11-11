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
Timer::Timer(const Time &time, bool periodic)
    : m_running(false),
      m_periodic(periodic),
      m_time(time),
      m_callback(defaultTimeoutCallback)
{
}
//=============================================================================
Timer::~Timer() {}
//=============================================================================
void Timer::setTime(const Time &time) { m_time = time; }
//=============================================================================
void Timer::start()
{
    auto &i = cerberus::Cerberus::instance();

    uint32_t time = m_time.microseconds() / 100;
    i.m_fastLoop.startTimer(m_running, time, m_periodic, m_callback);
}
//=============================================================================
void Timer::stop()
{
    auto &i = cerberus::Cerberus::instance();

    i.m_fastLoop.stopTimer(m_running);
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
