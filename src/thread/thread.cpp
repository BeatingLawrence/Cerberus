#include "thread.h"

#include <cstring>

#include "../cerberus.h"
#include "../exception/exception.h"

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

        switch (m_periodicity)
        {
            case TP_NonPeriodic:
            {
                if (isQueueEmpty())
                {
                    pause();
                }
                else
                {
                    m_retValue = tick();
                }
            }
            break;
            case TP_Periodic:
            {
                m_retValue = tick();
                wait();
            }
            break;
            case TP_PeriodicQueue:
            {
                m_retValue = tick();

                if (isQueueEmpty())
                {
                    wait();
                }
            }
            break;
            case TP_OneShot:
            {
                m_retValue = tick();
                terminate();
            }
            break;
            case TP_Continuos:
            {
                m_retValue = tick();
            }
            break;
        }
    }

    coolDown();
}
//=============================================================================
int cerberus::thread::Thread::defaultTickCallback(cerberus_message msg, Thread* thread) { return 0; }
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
void cerberus::thread::Thread::construct(ThreadPeriodicity periodicity, const time::TimeFrame& time, const std::string& name)
{
    if (periodicity == ThreadPeriodicity::TP_Periodic || periodicity == ThreadPeriodicity::TP_PeriodicQueue)
    {
        if (!time.isValid()) throw cerberusIllegalArgExc("Invalid time in Thread creation");

        m_time = time.splittedTime();
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

#ifdef LINUX_SYSTEM

    if (!name.empty()) pthread_setname_np(m_pthread, core::CerberusUtils::truncStr(name, 15).c_str());

#endif
}
//=============================================================================
int cerberus::thread::Thread::tick() { return m_tickCallback(nextMessage(), this); }
//=============================================================================
void cerberus::thread::Thread::warmUp() { m_warmUpCallback(); }
//=============================================================================
void cerberus::thread::Thread::coolDown() { m_coolDownCallback(); }
//=============================================================================
void cerberus::thread::Thread::sleep(const time::TimeFrame& time)
{
    auto splitted = time.splittedTime();
    timespec t{};
    t.tv_nsec = splitted.nanoseconds;
    t.tv_sec  = splitted.seconds;
    nanosleep(&t, NULL);
}
//=============================================================================
cerberus::thread::Thread::Thread(ThreadPeriodicity periodicity, const time::TimeFrame& time, const std::string& name)
    : ThreadBase(periodicity),
      CerberusObject(CerberusObject::ObjectType::Thread, name),
      m_pthread(),
      m_retValue(0),
      m_tickCallback(&defaultTickCallback),
      m_warmUpCallback(&defaultWarmUpCallback),
      m_coolDownCallback(&defaultCoolDownCallback)
{
    construct(periodicity, time, name);
}
//=============================================================================
cerberus::thread::Thread::Thread(const std::string& name)
    : ThreadBase(TP_NonPeriodic),
      CerberusObject(CerberusObject::ObjectType::Thread, name),
      m_pthread(),
      m_retValue(0),
      m_tickCallback(&defaultTickCallback),
      m_warmUpCallback(&defaultWarmUpCallback),
      m_coolDownCallback(&defaultCoolDownCallback)
{
    construct(TP_NonPeriodic, time::TimeFrame(), name);
}
//=============================================================================
cerberus::thread::Thread::Thread(ThreadPeriodicity periodicity, const std::string& name)
    : ThreadBase(periodicity),
      CerberusObject(CerberusObject::ObjectType::Thread, name),
      m_pthread(),
      m_retValue(0),
      m_tickCallback(&defaultTickCallback),
      m_warmUpCallback(&defaultWarmUpCallback),
      m_coolDownCallback(&defaultCoolDownCallback)
{
    construct(periodicity, time::TimeFrame(), name);
}
//=============================================================================
cerberus::thread::Thread::~Thread() { checkOut(); }
//=============================================================================
cerberus::time::SplittedTime cerberus::thread::Thread::getTime() const { return m_time; }
//=============================================================================
cerberus::OperationResult cerberus::thread::Thread::join(bool stop)
{
    if (stop)
    {
        terminate();  // set termination flag
        start();      // force the thread to run
    }

    int ret = pthread_join(m_pthread, NULL);

    if (ret)
    {
        if (ret == EINVAL)
        {
            return OR_ThreadNotJoinable;
        }
        else if (ret != ESRCH)  // ESRCH = not executing anymore
        {
            OperationResult toReturn(OR_Failure);
            toReturn.str = strerror(ret);
            return toReturn;
        }
    }

    return (int64_t)m_retValue;
}
//=============================================================================
cerberus::OperationResult cerberus::thread::Thread::detach()
{
    int ret = pthread_detach(m_pthread);

    if (ret)
    {
        OperationResult toReturn(OR_Failure);
        toReturn.str = strerror(ret);
        return toReturn;
    }

    return OR_OK;
}
//=============================================================================
void cerberus::thread::Thread::provideTickCallback(customTickCallback callback) { m_tickCallback = callback; }
//=============================================================================
void cerberus::thread::Thread::provideWarmUpCallback(customCallback callback) { m_warmUpCallback = callback; }
//=============================================================================
void cerberus::thread::Thread::provideCoolDownCallback(customCallback callback) { m_coolDownCallback = callback; }
//=============================================================================
