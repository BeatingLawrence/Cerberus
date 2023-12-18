#include "eventscheduler.h"

#include "src/mutex/mutexlocker.h"

using namespace cerberus::core;

//=============================================================================
int EventScheduler::tick()  // runs every 100us (0.1ms)
{
    mutex::MutexLocker locker(m_mutex);

    auto now = time::DateTime::current();

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
                (*it).callback();

                if ((*it).time.isValid())
                    // periodic timer
                    (*it).expiryDate = now.add((*it).time);
                else
                {
                    // one-shot timer
                    (*(*it).bit) = false;
                    m_timers.erase(it);
                    done = false;
                    break;
                }
            }
        }
    }

    return 0;
}
//=============================================================================
void EventScheduler::addTimer(std::atomic_bool &bit, time::DateTime d, time::TimeFrame t, timerCallback callback)
{
    TimerData data  = {};
    data.bit        = &bit;
    data.expiryDate = d;
    data.time       = t;
    data.callback   = callback;

    mutex::MutexLocker locker(m_mutex);

    for (auto &&el : m_timers)
    {
        if (el.bit == &bit)  // timer already in vector
        {
            (*el.bit)       = true;
            data.expiryDate = d;
            el.time         = t;
            el.callback     = callback;
            return;
        }
    }

    // not found

    (*data.bit) = true;
    m_timers.push_back(data);
}
//=============================================================================
EventScheduler::EventScheduler()
    : cerberus::thread::Thread(TP_Periodic, time::TimeFrame(100, time::TimeFrame::U_MicroSecond), "Event Scheduler")
{
}
//=============================================================================
EventScheduler::~EventScheduler()
{
    mutex::MutexLocker locker(m_mutex);

    for (auto &&el : m_timers)
    {
        (*el.bit) = false;
    }
}
//=============================================================================
void EventScheduler::startTimer(std::atomic_bool &bit, time::TimeFrame t, timerCallback callback) { addTimer(bit, time::DateTime::current().add(t), t, callback); }
//=============================================================================
void EventScheduler::startTimer(std::atomic_bool &bit, time::DateTime d, time::TimeFrame t, timerCallback callback) { addTimer(bit, d, t, callback); }
//=============================================================================
void EventScheduler::startTimer(std::atomic_bool &bit, time::DateTime d, timerCallback callback) { addTimer(bit, d, time::TimeFrame(), callback); }
//=============================================================================
void EventScheduler::stopTimer(std::atomic_bool &bit)
{
    mutex::MutexLocker locker(m_mutex);

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
