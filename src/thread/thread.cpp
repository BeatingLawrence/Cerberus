#include "thread.h"

#include <cstring>

#include "../cerberus.h"
#include "../exception/exception.h"

using namespace cerberus;

//=============================================================================
void* Thread::_staticThread(void* context)
{
    ((Thread*)context)->_thread();
    return nullptr;
}
//=============================================================================
void Thread::_thread()
{
    bool firstRun = true;

    while (true)
    {
        pause();

        if (getTerminateFlag()) break;

        if (firstRun)
        {
            warmUp();
            firstRun = false;
        }

        switch (m_periodicity)
        {
            case TP_Message:
            {
                if (isQueueEmpty())
                    stop();
                else
                    m_retValue = tick();
            }
            break;

            case TP_Periodic:
            {
                m_retValue = tick();
                wait();
            }
            break;

            case TP_PeriodicMessage:
            {
                m_retValue = tick();

                if (isQueueEmpty()) wait();
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

            case TP_Trigger:
            {
                m_retValue = tick();
                stop();
            }
            break;
        }
    }

    if (!firstRun) coolDown();  // cool down only if warmUp() was called

    dead();
}
//=============================================================================
int Thread::defaultTickCallback(cerberus_message msg, Thread* thread) { return 0; }
//=============================================================================
void Thread::defaultWarmUpCallback()
{
    // noop
}
//=============================================================================
void Thread::defaultCoolDownCallback()
{
    // noop
}
//=============================================================================
void Thread::wait()
{
    timespec t{};
    t.tv_nsec = m_time.nanoseconds;
    t.tv_sec  = m_time.seconds;
    nanosleep(&t, NULL);
}
//=============================================================================
void Thread::construct(ThreadPeriodicity periodicity, const TimeFrame& time, const std::string& name)
{
    if (periodicity == ThreadPeriodicity::TP_Periodic || periodicity == ThreadPeriodicity::TP_PeriodicMessage)
    {
        if (!time.isValid()) throw cIllegalArgExc("Invalid time in Thread creation");

        m_time = time.splittedTime();
    }

    pthread_attr_t attr{};

    if (pthread_attr_init(&attr))  // default attributes
    {
        throw cSystemExc("pthread_attr_init function failed");
    }

    auto ret = pthread_create(&m_pthread, &attr, &_staticThread, this);

    if (ret)
    {
        throw cSystemExc("pthread_create function failed: %s", strerror(ret));
    }

    pthread_attr_destroy(&attr);

#ifdef LINUX_SYSTEM

    if (!name.empty()) pthread_setname_np(m_pthread, CerberusUtils::truncStr(name, 15).c_str());

#endif
}
//=============================================================================
int Thread::tick() { return m_tickCallback(nextMessage().ref(), this); }
//=============================================================================
void Thread::warmUp() { m_warmUpCallback(); }
//=============================================================================
void Thread::coolDown() { m_coolDownCallback(); }
//=============================================================================
void Thread::sleep(const TimeFrame& time)
{
    auto splitted = time.splittedTime();
    timespec t{};
    t.tv_nsec = splitted.nanoseconds;
    t.tv_sec  = splitted.seconds;
    nanosleep(&t, NULL);
}
//=============================================================================
Thread::Thread(ThreadPeriodicity periodicity, const TimeFrame& time, const std::string& name)
    : ThreadBase(periodicity),
      CerberusObject(CerberusObject::ObjectType::COBJ_Thread, name),
      m_pthread(),
      m_retValue(0),
      m_tickCallback(&defaultTickCallback),
      m_warmUpCallback(&defaultWarmUpCallback),
      m_coolDownCallback(&defaultCoolDownCallback)
{
    construct(periodicity, time, name);
}
//=============================================================================
Thread::Thread(const std::string& name)
    : ThreadBase(TP_Message),
      CerberusObject(CerberusObject::ObjectType::COBJ_Thread, name),
      m_pthread(),
      m_retValue(0),
      m_tickCallback(&defaultTickCallback),
      m_warmUpCallback(&defaultWarmUpCallback),
      m_coolDownCallback(&defaultCoolDownCallback)
{
    construct(TP_Message, TimeFrame(), name);
}
//=============================================================================
Thread::Thread(ThreadPeriodicity periodicity, const std::string& name)
    : ThreadBase(periodicity),
      CerberusObject(CerberusObject::ObjectType::COBJ_Thread, name),
      m_pthread(),
      m_retValue(0),
      m_tickCallback(&defaultTickCallback),
      m_warmUpCallback(&defaultWarmUpCallback),
      m_coolDownCallback(&defaultCoolDownCallback)
{
    construct(periodicity, TimeFrame(), name);
}
//=============================================================================
Thread::~Thread() { checkOut(); }
//=============================================================================
SplittedTime Thread::getTime() const { return m_time; }
//=============================================================================
IntOpRes Thread::join(bool stop)
{
    if (isDead()) return m_retValue;

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
            IntOpRes toReturn(OR_Failure);
            toReturn.reason = strerror(ret);
            return toReturn;
        }
    }

    return m_retValue;
}
//=============================================================================
cerberus::OpRes Thread::detach()
{
    int ret = pthread_detach(m_pthread);

    if (ret)
    {
        OpRes toReturn(OR_Failure);
        toReturn.reason = strerror(ret);
        return toReturn;
    }

    return OR_OK;
}
//=============================================================================
void Thread::provideTickCallback(threadTickCallback callback) { m_tickCallback = callback; }
//=============================================================================
void Thread::provideWarmUpCallback(threadCallback callback) { m_warmUpCallback = callback; }
//=============================================================================
void Thread::provideCoolDownCallback(threadCallback callback) { m_coolDownCallback = callback; }
//=============================================================================
