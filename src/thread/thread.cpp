#include "thread.h"
#include <chrono>
#include <iostream>

#include "../mutex/mutexlocker.h"
#include "../exception/exceptioncatalog.h"
#include "../cerberus.h"

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
        if(getPausedFlag())     // paused
        {
            std::this_thread::yield();
        }
        else                    // execute
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
cerberus::thread::Thread::Thread(ThreadPeriodicity periodicity, const time::Time& time, const std::string& name) :
    ThreadBase(),
    m_thread(_staticThread, this),
    m_periodicity(periodicity),
    m_retValue(0),
    m_id(0)
{
    if(periodicity == ThreadPeriodicity::TP_Periodic && !time.isValid())
    {
        throw cerberusIllegalArgumentExc("cannot construct a periodic thread using an invalid time");
    }

    m_id = Cerberus::provider()->_registerThread(this, name);

    if(periodicity == ThreadPeriodicity::TP_NonPeriodic)
    {
        logInfo(Cerberus::strPrint("Creation of non-periodic Thread '%s' with ID: %u", name.c_str(), m_id));
    }
    else if(periodicity == ThreadPeriodicity::TP_Periodic)
    {
        m_period = std::chrono::microseconds(time.getMicroseconds());
        logInfo(Cerberus::strPrint("Creation of periodic Thread '%s' with ID: %u, period: %u ms", name.c_str(), m_id, time.getMilliseconds()));
    }
    else if(periodicity == ThreadPeriodicity::TP_OneShot)
    {
        logInfo(Cerberus::strPrint("Creation of one-shot Thread '%s' with ID: %u", name.c_str(), m_id));
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
