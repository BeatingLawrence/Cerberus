#include "eventscheduler.h"

#include "src/mutex/mutexlocker.h"

using namespace cerberus::core;

//=============================================================================
int EventScheduler::tick()  // runs every 100us (0.1ms)
{
    MutexLocker locker(m_mutex);

    auto now = DateTime::current();

    bool done = false;

    while (!done)
    {
        done = true;

        for (auto it = m_timers.begin(); it < m_timers.end(); it++)
        {
            if (*((*it).bit) == false)
            {
                continue;
            }

            if (now >= (*it).expiryDate)  // expired
            {
                if (!(*it).time.isNull())
                {
                    // periodic timer
                    (*it).expiryDate = now.add((*it).time);

                    (*it).callback((*it).ctx);  // call the callback
                }
                else
                {
                    // one-shot timer
                    (*(*it).bit) = false;
                    m_timers.erase(it);
                    done = false;

                    (*it).callback((*it).ctx);  // call the callback

                    break;
                }
            }
        }
    }

    return 0;
}
//=============================================================================
void EventScheduler::addTimer(std::atomic_bool &bit, DateTime d, TimeFrame t, timerCallback callback,
                              void *ctx)
{
    TimerData data  = {};
    data.bit        = &bit;
    data.expiryDate = d;
    data.time       = t;
    data.callback   = callback;
    data.ctx        = ctx;

    MutexLocker locker(m_mutex);

    for (auto &&el : m_timers)
    {
        if (el.bit == &bit)  // timer already in vector
        {
            (*el.bit)       = true;
            data.expiryDate = d;
            el.time         = t;
            el.callback     = callback;
            el.ctx          = ctx;
            return;
        }
    }

    // not found

    (*data.bit) = true;
    m_timers.push_back(data);
}
//=============================================================================
EventScheduler::EventScheduler()
    : cerberus::Thread(TP_Periodic, TimeFrame(100, TimeFrame::U_MicroSecond), "Event Scheduler")
{
}
//=============================================================================
EventScheduler::~EventScheduler()
{
    MutexLocker locker(m_mutex);

    for (auto &&el : m_timers)
    {
        (*el.bit) = false;
    }
}
//=============================================================================
void EventScheduler::startTimer(std::atomic_bool &bit, TimeFrame t, timerCallback callback, void *ctx)
{
    addTimer(bit, DateTime::current().add(t), t, callback, ctx);
}
//=============================================================================
void EventScheduler::startTimer(std::atomic_bool &bit, DateTime d, TimeFrame t, timerCallback callback,
                                void *ctx)
{
    addTimer(bit, d, t, callback, ctx);
}
//=============================================================================
void EventScheduler::startTimer(std::atomic_bool &bit, DateTime d, timerCallback callback, void *ctx)
{
    addTimer(bit, d, TimeFrame(), callback, ctx);
}
//=============================================================================
void EventScheduler::stopTimer(std::atomic_bool &bit)
{
    MutexLocker locker(m_mutex);

    for (auto it = m_timers.begin(); it < m_timers.end(); it++)
    {
        if (it->bit == &bit)
        {
            m_timers.erase(it);
            bit = false;
            return;
        }
    }
}
//=============================================================================
