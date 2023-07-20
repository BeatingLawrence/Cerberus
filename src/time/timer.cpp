#include "timer.h"

#include <cerrno>
#include <cstring>

#include "src/core/cerberuslog.h"

using namespace cerberus::time;

//=============================================================================
void Timer::defaultTimeoutCallback()
{
    // noop
}
//=============================================================================
void Timer::mainCallback(sigval val)
{
    ((Timer *)val.sival_ptr)->m_running = false;
    ((Timer *)val.sival_ptr)->m_callback();
}
//=============================================================================
Timer::Timer()
    : m_callback(&defaultTimeoutCallback),
      m_running(false),
      m_timerId(0),
      m_failed(false),
      m_periodic(false),
      m_time()
{
    sigevent event{};

    event.sigev_notify = SIGEV_THREAD;
    event.sigev_notify_function = &mainCallback;
    event.sigev_value.sival_ptr = this;

    if (timer_create(CLOCK_MONOTONIC, &event, &m_timerId) == -1)
    {
        debug("error in timer_create: %s", strerror(errno));
        m_failed = true;
    }
}
//=============================================================================
Timer::~Timer()
{
    if (timer_delete(m_timerId) == -1)
    {
        debug("error in timer_delete: %s", strerror(errno));
    }
}
//=============================================================================
Timer::Timer(const Time &time, bool periodic)
    : m_callback(&defaultTimeoutCallback),
      m_running(false),
      m_timerId(0),
      m_failed(false),
      m_periodic(periodic),
      m_time(time)
{
    sigevent event{};

    event.sigev_notify = SIGEV_THREAD;
    event.sigev_notify_function = &mainCallback;
    event.sigev_value.sival_ptr = this;

    if (timer_create(CLOCK_MONOTONIC, &event, &m_timerId) == -1)
    {
        debug("error in timer_create: %s", strerror(errno));
        m_failed = true;
    }
}
//=============================================================================
void Timer::setTime(const Time &time) { m_time = time; }
//=============================================================================
void Timer::start()
{
    itimerspec spec{};
    auto split = m_time.splittedTime();
    spec.it_value.tv_nsec = split.nanoseconds;
    spec.it_value.tv_sec = split.seconds;

    if (m_periodic)
    {
        spec.it_interval.tv_nsec = spec.it_value.tv_nsec;
        spec.it_interval.tv_sec = spec.it_value.tv_sec;
    }

    if (timer_settime(m_timerId, 0, &spec, nullptr) == -1)
    {
        debug("error in timer_settime: %s", strerror(errno));
        m_failed = true;
    }
    else
    {
        m_running = true;
    }
}
//=============================================================================
void Timer::stop()
{
    itimerspec spec{};

    if (timer_settime(m_timerId, 0, &spec, nullptr) == -1)
    {
        debug("error in timer_settime: %s", strerror(errno));
        m_failed = true;
    }
    else
    {
        m_running = false;
    }
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
bool Timer::isFailed() { return m_failed; }
//=============================================================================
void Timer::provideTimeoutCallback(timerCallback callback) { m_callback = callback; }
//=============================================================================
