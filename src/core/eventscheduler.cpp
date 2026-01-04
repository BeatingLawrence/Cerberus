#include "eventscheduler.h"

#include "src/thread/mutexlocker.h"

using namespace cerberus::core;

//=============================================================================
EventScheduler::EventScheduler()
    : cerberus::Thread(TP_Periodic, TimeFrame(100, TimeFrame::U_MicroSecond), "Event Scheduler")
{
}
//=============================================================================
EventScheduler::~EventScheduler()
{
    MutexLocker locker(m_mutex);

    for (auto& el : m_timers) el.bit->store(false, std::memory_order_relaxed);

    m_timers.clear();
}
//=============================================================================
int EventScheduler::tick()  // runs every 100us (0.1ms)
{
    std::vector<std::pair<timerCallback, void*>> calls;
    calls.reserve(8);

    const DateTime now = DateTime::current();

    {
        MutexLocker locker(m_mutex);

        for (auto it = m_timers.begin(); it != m_timers.end();)
        {
            if (!it->bit->load(std::memory_order_relaxed))
            {
                ++it;
                continue;
            }

            if (now < it->delay)
            {
                ++it;
                continue;
            }

            auto cb  = it->callback;
            auto ctx = it->ctx;
            if (it->expired)
                it->expired->store(true, std::memory_order_relaxed);

            if (!it->time.isNull())
            {
                DateTime next = now;
                next.add(it->time);
                it->delay = next;

                calls.emplace_back(cb, ctx);
                ++it;
            }
            else
            {
                it->bit->store(false, std::memory_order_relaxed);
                it = m_timers.erase(it);

                calls.emplace_back(cb, ctx);
            }
        }
    }  // unlock

    for (auto& [cb, ctx] : calls) cb(ctx);

    return 0;
}
//=============================================================================
void EventScheduler::addTimer(std::atomic_bool* bit, std::atomic_bool* expired, DateTime d, TimeFrame t,
                              timerCallback callback, void* ctx)
{
    MutexLocker locker(m_mutex);

    for (auto& el : m_timers)
    {
        if (el.bit == bit)
        {
            el.delay    = d;
            el.time     = t;
            el.expired  = expired;
            el.callback = callback;
            el.ctx      = ctx;
            el.bit->store(true, std::memory_order_relaxed);
            return;
        }
    }

    TimerData data = {};
    data.bit       = bit;
    data.expired   = expired;
    data.delay     = d;
    data.time      = t;
    data.callback  = callback;
    data.ctx       = ctx;

    data.bit->store(true, std::memory_order_relaxed);
    m_timers.push_back(data);
}
//=============================================================================
void EventScheduler::startTimer(TimerData& data)
{
    if (!data.isValid()) return;

    if (data.isPeriodic())
    {
        if (data.isDelayed())
            addTimer(data.bit, data.expired, data.delay, data.time, data.callback, data.ctx);
        else
        {
            DateTime d = DateTime::current();
            d.add(data.time);
            addTimer(data.bit, data.expired, d, data.time, data.callback, data.ctx);
        }
    }
    else
    {
        addTimer(data.bit, data.expired, data.delay, TimeFrame(), data.callback, data.ctx);
    }
}
//=============================================================================
void EventScheduler::stopTimer(std::atomic_bool& bit)
{
    MutexLocker locker(m_mutex);

    for (auto it = m_timers.begin(); it != m_timers.end(); ++it)
    {
        if (it->bit == &bit)
        {
            m_timers.erase(it);
            bit.store(false, std::memory_order_relaxed);
            return;
        }
    }

    bit.store(false, std::memory_order_relaxed);
}
//=============================================================================
