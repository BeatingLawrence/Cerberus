#include "thread.h"

#include <cstring>
#include <iostream>

#include "../core/cerberuslog.h"
#include "../exception/exceptioncatalog.h"

//=============================================================================
void* cerberus::thread::Thread::_staticThread(void* context)
{
    ((Thread*)context)->_thread();
    return nullptr;
}
//=============================================================================
void cerberus::thread::Thread::_thread()
{
    bool firstRun = true;

    while (!getTerminateFlag())
    {
        pause();

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
                // yield
            }
            else
            {
                m_retValue = tick();
            }
        }
        else if (m_periodicity == TP_Periodic)
        {
            m_retValue = tick();

            wait();
        }
        else if (m_periodicity == TP_PeriodicQueue)
        {
            m_retValue = tick();

            if (isQueueEmpty())
            {
                wait();
            }
        }
    }

    coolDown();
}
//=============================================================================
int cerberus::thread::Thread::defaultTickCallback(message::cerberus_message msg, Thread* thread) { return 0; }
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
void cerberus::thread::Thread::wait()
{
    timespec t{};
    t.tv_nsec = m_time.nanoseconds;
    t.tv_sec  = m_time.seconds;
    nanosleep(&t, NULL);
}
//=============================================================================
int cerberus::thread::Thread::tick() { return m_tickCallback(nextMessage(), this); }
//=============================================================================
void cerberus::thread::Thread::warmUp() { m_warmUpCallback(); }
//=============================================================================
void cerberus::thread::Thread::coolDown() { m_coolDownCallback(); }
//=============================================================================
void cerberus::thread::Thread::sleep(const time::Time& time)
{
    auto splitted = time.splittedTime();
    timespec t{};
    t.tv_nsec = splitted.nanoseconds;
    t.tv_sec  = splitted.seconds;
    nanosleep(&t, NULL);
}
//=============================================================================
cerberus::thread::Thread::Thread(const std::string& name, ThreadPeriodicity periodicity, const time::Time& time)
    : ThreadBase(),
      CerberusObject(CerberusObject::ObjectType::Thread, name),
      m_pthread(),
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
            m_time = time.splittedTime();
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

    pthread_attr_t attr{};

    if (pthread_attr_init(&attr))  // default attributes
    {
        throw cerberusSystemExc("pthread_attr_init function failed");
    }

    auto ret = pthread_create(&m_pthread, &attr, &_staticThread, this);

    if (ret)
    {
        throw cerberusSystemExc("pthread_create function failed: %s", strerror(ret));
    }

    pthread_attr_destroy(&attr);
}
//=============================================================================
cerberus::thread::Thread::~Thread() {}
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

    int ret = pthread_join(m_pthread, NULL);

    if (ret)
    {
        if (ret == EINVAL)
        {
            logError("Thread not joinable");
            return 0;
        }
        else if (ret == ESRCH)  // not executing anymore
        {
            return m_retValue;
        }

        throw cerberusSystemExc("pthread_join function failed: %s", strerror(ret));
    }

    return m_retValue;
}
//=============================================================================
void cerberus::thread::Thread::detach()
{
    int ret = pthread_detach(m_pthread);

    if (ret)
    {
        throw cerberusSystemExc("pthread_detach function failed: %s", strerror(ret));
    }
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
