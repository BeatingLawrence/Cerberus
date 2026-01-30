#include "systimer.h"
#ifdef LINUX_SYSTEM
#include <cstring>

#include "src/cerberus.h"

using namespace crb::time;

//=============================================================================
void SysTimer::defaultTimeoutCallback(void *ctx)
{
    (void)ctx;
    // noop
}
//=============================================================================
void SysTimer::mainCallback(sigval val)
{
    ((SysTimer *)val.sival_ptr)->m_running = false;
    ((SysTimer *)val.sival_ptr)->m_callback(((SysTimer *)val.sival_ptr)->m_ctx);
}
//=============================================================================
SysTimer::SysTimer()
    : m_callback(&defaultTimeoutCallback),
      m_running(false),
      m_timerId(0),
      m_failed(false),
      m_periodic(false),
      m_time()
{
    sigevent event{};

    event.sigev_notify          = SIGEV_THREAD;
    event.sigev_notify_function = &mainCallback;
    event.sigev_value.sival_ptr = this;

    if (timer_create(CLOCK_MONOTONIC, &event, &m_timerId) == -1)
    {
        logDebug("error in timer_create: %s", strerror(errno));
        m_failed = true;
    }
}
//=============================================================================
SysTimer::~SysTimer()
{
    if (timer_delete(m_timerId) == -1)
    {
        logDebug("error in timer_delete: %s", strerror(errno));
    }
}
//=============================================================================
SysTimer::SysTimer(const TimeFrame &time, bool periodic)
    : m_callback(&defaultTimeoutCallback),
      m_ctx(),
      m_running(false),
      m_timerId(0),
      m_failed(false),
      m_periodic(periodic),
      m_time(time)
{
    sigevent event{};

    event.sigev_notify          = SIGEV_THREAD;
    event.sigev_notify_function = &mainCallback;
    event.sigev_value.sival_ptr = this;

    if (timer_create(CLOCK_MONOTONIC, &event, &m_timerId) == -1)
    {
        logDebug("error in timer_create: %s", strerror(errno));
        m_failed = true;
    }
}
//=============================================================================
void SysTimer::setTime(const TimeFrame &time) { m_time = time; }
//=============================================================================
void SysTimer::start()
{
    itimerspec spec{};
    auto split            = m_time.splittedTime();
    spec.it_value.tv_nsec = split.nanoseconds;
    spec.it_value.tv_sec  = split.seconds;

    if (m_periodic)
    {
        spec.it_interval.tv_nsec = spec.it_value.tv_nsec;
        spec.it_interval.tv_sec  = spec.it_value.tv_sec;
    }

    if (timer_settime(m_timerId, 0, &spec, nullptr) == -1)
    {
        logDebug("error in timer_settime: %s", strerror(errno));
        m_failed = true;
    }
    else
    {
        m_running = true;
    }
}
//=============================================================================
void SysTimer::stop()
{
    itimerspec spec{};

    if (timer_settime(m_timerId, 0, &spec, nullptr) == -1)
    {
        logDebug("error in timer_settime: %s", strerror(errno));
        m_failed = true;
    }
    else
    {
        m_running = false;
    }
}
//=============================================================================
void SysTimer::reset()
{
    stop();
    start();
}
//=============================================================================
bool SysTimer::isRunning() { return m_running; }
//=============================================================================
bool SysTimer::isFailed() { return m_failed; }
//=============================================================================
void SysTimer::provideTimeoutCallback(timerCallback callback, void *ctx) { m_callback = callback; }
//=============================================================================
#endif
