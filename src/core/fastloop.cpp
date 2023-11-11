#include "fastloop.h"

#include "src/mutex/mutexlocker.h"

using namespace cerberus::core;

//=============================================================================
int FastLoop::tick()  // runs every 100us (0.1ms)
{
    mutex::MutexLocker locker(m_mutex);

    for (auto &&el : m_timers)
    {
        el.time++;
    }

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

            if ((*it).time == (*it).target)
            {
                (*it).time = 0;
                (*it).callback();

                if (!(*it).periodic)
                {
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
FastLoop::FastLoop()
    : cerberus::thread::Thread(TP_Periodic, {100, time::Time::U_MicroSecond})
{
}
//=============================================================================
FastLoop::~FastLoop() {}
//=============================================================================
void FastLoop::startTimer(std::atomic_bool &bit, uint32_t target, bool periodic, timerCallback callback)
{
    TimerData data = {};
    data.bit       = &bit;
    data.target    = target;
    data.periodic  = periodic;
    data.callback  = callback;

    mutex::MutexLocker locker(m_mutex);

    for (auto &&el : m_timers)
    {
        if (el.bit == &bit)  // timer already in vector
        {
            (*el.bit)   = true;
            el.time     = 0;
            el.periodic = periodic;
            el.target   = target;
            el.callback = callback;
            return;
        }
    }

    (*data.bit) = true;
    m_timers.push_back(data);
}
//=============================================================================
void FastLoop::stopTimer(std::atomic_bool &bit)
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
