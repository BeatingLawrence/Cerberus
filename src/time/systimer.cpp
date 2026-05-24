#include "systimer.h"
#ifdef LINUX_SYSTEM
#include <cerrno>
#include <cstring>
#include <cstdint>

#include "src/cerberus.h"

using namespace crb::time;

namespace
{
uint64_t toNs(const crb::TimeFrame& time)
{
    return time.toMicroseconds() * 1000u;
}

uint64_t toNs(const timespec& ts)
{
    return static_cast<uint64_t>(ts.tv_sec) * 1000000000ull +
           static_cast<uint64_t>(ts.tv_nsec);
}

timespec fromNs(uint64_t ns)
{
    timespec ts{};
    ts.tv_sec = static_cast<time_t>(ns / 1000000000ull);
    ts.tv_nsec = static_cast<long>(ns % 1000000000ull);
    return ts;
}
}  // namespace

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
bool SysTimer::ensureTimer()
{
    if (m_timerCreated) return true;

    sigevent event{};

    event.sigev_notify          = SIGEV_THREAD;
    event.sigev_notify_function = &mainCallback;
    event.sigev_value.sival_ptr = this;

    if (timer_create(CLOCK_MONOTONIC, &event, &m_timerId) == -1)
    {
        logDebug("error in timer_create: %s", strerror(errno));
        m_failed = true;
        return false;
    }

    m_timerCreated = true;
    return true;
}
//=============================================================================
SysTimer::SysTimer()
    : m_callback(&defaultTimeoutCallback),
      m_ctx(nullptr),
      m_running(false),
      m_timerId(0),
      m_timerCreated(false),
      m_failed(false),
      m_periodic(false),
      m_time(),
      m_periodNs(0),
      m_nextDeadline{},
      m_deadlineArmed(false),
      m_overrun(false)
{
}
//=============================================================================
SysTimer::~SysTimer()
{
    if (!m_timerCreated) return;
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
      m_timerCreated(false),
      m_failed(false),
      m_periodic(periodic),
      m_time(time),
      m_periodNs(toNs(time)),
      m_nextDeadline{},
      m_deadlineArmed(false),
      m_overrun(false)
{
}
//=============================================================================
void SysTimer::setTime(const TimeFrame &time)
{
    m_time = time;
    m_periodNs = toNs(time);
    m_deadlineArmed = false;
    m_overrun = false;
}
//=============================================================================
void SysTimer::start()
{
    if (!ensureTimer()) return;

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
    if (!m_timerCreated)
    {
        m_running = false;
        return;
    }

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
void SysTimer::startDeadline()
{
    if (m_periodNs == 0)
    {
        m_deadlineArmed = false;
        m_overrun = false;
        return;
    }

    timespec now{};
    if (clock_gettime(CLOCK_MONOTONIC, &now) == -1)
    {
        logDebug("error in clock_gettime: %s", strerror(errno));
        m_failed = true;
        m_deadlineArmed = false;
        m_overrun = false;
        return;
    }

    m_nextDeadline = fromNs(toNs(now) + m_periodNs);
    m_deadlineArmed = true;
    m_overrun = false;
}
//=============================================================================
bool SysTimer::waitDeadline()
{
    if (m_periodNs == 0)
    {
        m_overrun = false;
        return m_overrun;
    }

    if (!m_deadlineArmed) startDeadline();
    if (!m_deadlineArmed) return m_overrun;

    timespec now{};
    if (clock_gettime(CLOCK_MONOTONIC, &now) == -1)
    {
        logDebug("error in clock_gettime: %s", strerror(errno));
        m_failed = true;
        m_overrun = false;
        return m_overrun;
    }

    const uint64_t nowNs = toNs(now);
    uint64_t deadlineNs = toNs(m_nextDeadline);

    m_overrun = (nowNs >= deadlineNs);

    if (!m_overrun)
    {
        int ret = 0;
        do
        {
            ret = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &m_nextDeadline, nullptr);
        } while (ret == EINTR);

        if (ret != 0)
        {
            logDebug("error in clock_nanosleep: %s", strerror(ret));
            m_failed = true;
        }
    }

    do
    {
        deadlineNs += m_periodNs;
    } while (deadlineNs <= nowNs);

    m_nextDeadline = fromNs(deadlineNs);
    return m_overrun;
}
//=============================================================================
bool SysTimer::isRunning() { return m_running; }
//=============================================================================
bool SysTimer::isFailed() { return m_failed; }
//=============================================================================
bool SysTimer::isOverrun() const { return m_overrun; }
//=============================================================================
void SysTimer::provideTimeoutCallback(timerCallback callback, void *ctx)
{
    m_callback = callback ? callback : &defaultTimeoutCallback;
    m_ctx = ctx;
}
//=============================================================================
#else
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
    (void)val;
    // noop
}
//=============================================================================
bool SysTimer::ensureTimer()
{
    m_failed = true;
    return false;
}
//=============================================================================
SysTimer::SysTimer()
    : m_callback(&defaultTimeoutCallback),
      m_ctx(nullptr),
      m_running(false),
      m_timerId(0),
      m_timerCreated(false),
      m_failed(false),
      m_periodic(false),
      m_time(),
      m_periodNs(0),
      m_nextDeadline{},
      m_deadlineArmed(false),
      m_overrun(false)
{
}
//=============================================================================
SysTimer::~SysTimer() {}
//=============================================================================
SysTimer::SysTimer(const TimeFrame &time, bool periodic)
    : m_callback(&defaultTimeoutCallback),
      m_ctx(nullptr),
      m_running(false),
      m_timerId(0),
      m_timerCreated(false),
      m_failed(false),
      m_periodic(periodic),
      m_time(time),
      m_periodNs(time.toMicroseconds() * 1000u),
      m_nextDeadline{},
      m_deadlineArmed(false),
      m_overrun(false)
{
}
//=============================================================================
void SysTimer::setTime(const TimeFrame &time)
{
    m_time = time;
    m_periodNs = time.toMicroseconds() * 1000u;
    m_deadlineArmed = false;
    m_overrun = false;
}
//=============================================================================
void SysTimer::start()
{
    m_failed = true;
    m_running = false;
}
//=============================================================================
void SysTimer::stop() { m_running = false; }
//=============================================================================
void SysTimer::reset()
{
    stop();
    start();
}
//=============================================================================
void SysTimer::startDeadline()
{
    m_failed = true;
    m_deadlineArmed = false;
    m_overrun = false;
}
//=============================================================================
bool SysTimer::waitDeadline()
{
    m_failed = true;
    m_overrun = false;
    return m_overrun;
}
//=============================================================================
bool SysTimer::isRunning() { return m_running; }
//=============================================================================
bool SysTimer::isFailed() { return m_failed; }
//=============================================================================
bool SysTimer::isOverrun() const { return m_overrun; }
//=============================================================================
void SysTimer::provideTimeoutCallback(timerCallback callback, void *ctx)
{
    m_callback = callback ? callback : &defaultTimeoutCallback;
    m_ctx = ctx;
}
//=============================================================================
#endif
