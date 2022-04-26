#include "thread.h"
#include <chrono>
#include <iostream>

#include "../mutex/mutexlocker.h"
#include "../exception/exceptioncatalog.h"

//=============================================================================
void cerberus::thread::Thread::_staticThread(Thread* context)
{
    context->_thread();
}
//=============================================================================
void cerberus::thread::Thread::_thread()
{
    bool firstRun = true;

    while(!getTerminateFlag())
    {
        if(getPausedFlag())
        {
            std::this_thread::yield();
        }
        else
        {
            if(firstRun)
            {
                warmUp();
                firstRun = false;
            }

            if(m_periodicity == TP_OneShot)
            {
                m_retValue = tick();
                setTerminateFlag(true);
            }
            else if(m_periodicity == TP_NonPeriodic)
            {
                if(isQueueEmpty())
                {
                    std::this_thread::yield();
                }
                else
                {
                    m_retValue = tick();
                }
            }
            else if(m_periodicity == TP_Periodic)
            {
                m_retValue = tick();
                std::this_thread::sleep_for(m_period);
            }
        }
    }

    coolDown();
}
//=============================================================================
int cerberus::thread::Thread::tick()
{
    std::this_thread::yield();
    return 0;
}
//=============================================================================
void cerberus::thread::Thread::warmUp()
{
    // noop
}
//=============================================================================
void cerberus::thread::Thread::coolDown()
{
    // noop
}
//=============================================================================
void cerberus::thread::Thread::sleep(const time::Time& time)
{
    std::this_thread::sleep_for(std::chrono::microseconds(time.getMicroseconds()));
}
//=============================================================================
cerberus::thread::Thread::Thread(ThreadPeriodicity periodicity, const time::Time& time) :
    ThreadBase(),
    m_thread(_staticThread, this),
    m_periodicity(periodicity),
    m_retValue(0)
{
    if(periodicity == ThreadPeriodicity::TP_NonPeriodic)
    {
        //log non periodic thread creation
    }
    else if(periodicity == ThreadPeriodicity::TP_Periodic)
    {
        //log periodic thread creation
        if(time.isValid())
        {
            m_period = std::chrono::microseconds(time.getMicroseconds());
        }
        else
        {
            throw cerberusIllegalArgumentExc("cannot construct a periodic thread using an invalid time");
        }
    }
    else if(periodicity == ThreadPeriodicity::TP_OneShot)
    {
        //log oneshot thread creation
    }
    else
    {
        throw cerberusIllegalArgumentExc("invalid periodicity specifier");
    }
}
//=============================================================================
cerberus::thread::Thread::~Thread()
{
    join(!getTerminateFlag());
}
//=============================================================================
void cerberus::thread::Thread::start()
{
    setPausedFlag(false);
}
//=============================================================================
void cerberus::thread::Thread::stop()
{
    setPausedFlag(true);
}
//=============================================================================
int cerberus::thread::Thread::join(bool stop)
{
    if(stop && !getTerminateFlag())
    {
        terminate();
    }

    if(m_thread.joinable())
    {
        m_thread.join();
    }

    return m_retValue;
}
//=============================================================================
void cerberus::thread::Thread::terminate()
{
    setTerminateFlag(true);
}
//=============================================================================
