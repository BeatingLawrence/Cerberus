#include "thread.h"

#include <chrono>
#include <iostream>

#include "../core/cerberuslog.h"
#include "../exception/exceptioncatalog.h"

//=============================================================================
void cerberus::thread::Thread::_staticThread(Thread* context) { context->_thread(); }
//=============================================================================
void cerberus::thread::Thread::_thread()
{
    bool firstRun = true;

    while (!getTerminateFlag())
    {
        if (getPausedFlag())  // paused
        {
            std::this_thread::yield();
        }
        else  // execute
        {
            if (firstRun)
            {
                warmUp();
                firstRun = false;
            }

            if (m_periodicity == TP_OneShot)
            {
                m_retValue = tick();
                setTerminateFlag(true);
            }
            else if (m_periodicity == TP_NonPeriodic)
            {
                if (isQueueEmpty())
                {
                    std::this_thread::yield();
                }
                else
                {
                    m_retValue = tick();
                }
            }
            else if (m_periodicity == TP_Periodic)
            {
                m_retValue = tick();
                std::this_thread::sleep_for(m_period);
            }
            else if (m_periodicity == TP_PeriodicQueue)
            {
                m_retValue = tick();

                if (isQueueEmpty())
                {
                    std::this_thread::sleep_for(m_period);
                }
            }
        }
    }

    coolDown();
}
//=============================================================================
int cerberus::thread::Thread::defaultTickCallback(message::cerberus_message msg, Thread* thread)
{
    std::this_thread::yield();
    return 0;
}
//=============================================================================
void cerberus::thread::Thread::defaultWarmUpCallback()
{
    // noop
}
//=============================================================================
void cerberus::thread::Thread::defaultCoolDownCallback()
{
    // noop
}
//=============================================================================
int cerberus::thread::Thread::tick() { return m_tickCallback(nextMessage(), this); }
//=============================================================================
void cerberus::thread::Thread::warmUp() { m_warmUpCallback(); }
//=============================================================================
void cerberus::thread::Thread::coolDown() { m_coolDownCallback(); }
//=============================================================================
void cerberus::thread::Thread::sleep(const time::Time& time) { std::this_thread::sleep_for(std::chrono::microseconds(time.microseconds())); }
//=============================================================================
cerberus::thread::Thread::Thread(const std::string& name, ThreadPeriodicity periodicity, const time::Time& time)
    : ThreadBase(),
      CerberusObject(CerberusObject::ObjectType::Thread, name),
      m_thread(_staticThread, this),
      m_periodicity(periodicity),
      m_retValue(0),
      m_tickCallback(&defaultTickCallback),
      m_warmUpCallback(&defaultWarmUpCallback),
      m_coolDownCallback(&defaultCoolDownCallback)
{
    if (periodicity == ThreadPeriodicity::TP_NonPeriodic)
    {
        debug("New non-periodic %s", toObjStr().c_str());
    }
    else if (periodicity == ThreadPeriodicity::TP_Periodic || periodicity == ThreadPeriodicity::TP_PeriodicQueue)
    {
        if (time.isValid())
        {
            m_period = std::chrono::microseconds(time.microseconds());
            debug("New periodic %s, PERIOD:%ums", toObjStr().c_str(), time.milliseconds());
        }
        else
        {
            throw cerberusIllegalArgExc("Invalid time in Thread creation");
        }
    }
    else if (periodicity == ThreadPeriodicity::TP_OneShot)
    {
        debug("New one-shot %s", toObjStr().c_str());
    }
}
//=============================================================================
cerberus::thread::Thread::~Thread() { join(true); }
//=============================================================================
void cerberus::thread::Thread::start() { setPausedFlag(false); }
//=============================================================================
void cerberus::thread::Thread::stop() { setPausedFlag(true); }
//=============================================================================
int cerberus::thread::Thread::join(bool stop)
{
    if (stop)
    {
        terminate();
    }

    if (m_thread.joinable())
    {
        m_thread.join();
    }

    return m_retValue;
}
//=============================================================================
void cerberus::thread::Thread::terminate() { setTerminateFlag(true); }
//=============================================================================
void cerberus::thread::Thread::provideTickCallback(customTickCallback callback) { m_tickCallback = callback; }
//=============================================================================
void cerberus::thread::Thread::provideWarmUpCallback(customCallback callback) { m_warmUpCallback = callback; }
//=============================================================================
void cerberus::thread::Thread::provideCoolDownCallback(customCallback callback) { m_coolDownCallback = callback; }
//=============================================================================
