#include "thread.h"

#include <cstring>

#include "../exception/exception.h"
#include "../message/message.h"  // IWYU pragma: export

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

        resetRescheduling();

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
                if (hasMessage())
                    m_retValue = tick();
                else
                    stop();
            }
            break;

            case TP_Periodic:
            {
                m_retValue = tick();
                _wait();
            }
            break;

            case TP_PeriodicMessage:
            {
                m_retValue = tick();
                if (!hasMessage()) _wait();
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
int Thread::defaultTickCallback(msg_ptr msg, Thread* thread) { return 0; }
//=============================================================================
void Thread::defaultWarmUpCallback(Thread* thread)
{
    (void)thread;
    // noop
}
//=============================================================================
void Thread::defaultCoolDownCallback(Thread* thread)
{
    (void)thread;
    // noop
}
//=============================================================================
void Thread::_wait()
{
    if (isRescheduling()) return;  // bypass wait if thread is rescheduling
    timespec t{};
    t.tv_nsec = m_time.nanoseconds;
    t.tv_sec  = m_time.seconds;
    nanosleep(&t, NULL);
}
//=============================================================================
void Thread::_stopIfNoMessage()
{
    bool empty = _lockAndCheckEmpty();
    if (empty) stop();
    _unlock();
}
//=============================================================================
void Thread::_construct(ThreadPeriodicity periodicity, const TimeFrame& time, const std::string& name)
{
    if (periodicity == ThreadPeriodicity::TP_Periodic || periodicity == ThreadPeriodicity::TP_PeriodicMessage)
    {
        if (time.isNull()) throw cIllegalArgExc("Invalid time in Thread creation");

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
int Thread::tick() { return m_tickCallback(next(), this); }
//=============================================================================
void Thread::warmUp() { m_warmUpCallback(this); }
//=============================================================================
void Thread::coolDown() { m_coolDownCallback(this); }
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
    : ThreadBase(periodicity, name),
      m_pthread(),
      m_retValue(0),
      m_tickCallback(&defaultTickCallback),
      m_warmUpCallback(&defaultWarmUpCallback),
      m_coolDownCallback(&defaultCoolDownCallback)
{
    _construct(periodicity, time, name);
}
//=============================================================================
Thread::Thread(const std::string& name)
    : ThreadBase(TP_Message, name),
      m_pthread(),
      m_retValue(0),
      m_tickCallback(&defaultTickCallback),
      m_warmUpCallback(&defaultWarmUpCallback),
      m_coolDownCallback(&defaultCoolDownCallback)
{
    _construct(TP_Message, TimeFrame(), name);
}
//=============================================================================
Thread::Thread(ThreadPeriodicity periodicity, const std::string& name)
    : ThreadBase(periodicity, name),
      m_pthread(),
      m_retValue(0),
      m_tickCallback(&defaultTickCallback),
      m_warmUpCallback(&defaultWarmUpCallback),
      m_coolDownCallback(&defaultCoolDownCallback)
{
    _construct(periodicity, TimeFrame(), name);
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
