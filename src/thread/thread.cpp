#include "thread.h"

#include <limits.h>
#include <pthread.h>
#include <sched.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

#include "../cerberus.h"
#include "../exception/exception.h"
#include "../message/message.h"  // IWYU pragma: export

using namespace cerberus;

CoreSet Thread::s_defaultCoreSet;

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
                if (hasMessage()) m_retValue = tick();
                queueCheckStop();
            }
            break;

            case TP_Periodic:
            case TP_Periodic_realtime:
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
void Thread::_construct(ThreadPeriodicity periodicity, const TimeFrame& time, LSIZE stackSize,
                        const CoreSet& coreSet)
{
    if (periodicity == ThreadPeriodicity::TP_Periodic ||
        periodicity == ThreadPeriodicity::TP_Periodic_realtime ||
        periodicity == ThreadPeriodicity::TP_PeriodicMessage)
    {
        if (time.isNull()) throw cIllegalArgExc("Invalid time in Thread creation");
        m_time = time.splittedTime();
    }

    pthread_attr_t attr{};

    if (pthread_attr_init(&attr))  // default attributes
        throw cSystemExc("pthread_attr_init function failed");

    const bool isRealtime = (periodicity == ThreadPeriodicity::TP_Periodic_realtime);

    if (stackSize > 0 && stackSize < PTHREAD_STACK_MIN)
    {
        pthread_attr_destroy(&attr);
        throw cIllegalArgExc("Invalid stack size: below PTHREAD_STACK_MIN");
    }

    if (isRealtime)
    {
        int attrRet = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

        if (attrRet)
        {
            pthread_attr_destroy(&attr);
            throw cSystemExc("pthread_attr_setinheritsched function failed: %s", strerror(attrRet));
        }
    }

    CoreSet effectiveCoreSet = coreSet;
    if (effectiveCoreSet.empty() && !s_defaultCoreSet.empty())
    {
        effectiveCoreSet = s_defaultCoreSet;
    }

    if (!effectiveCoreSet.empty())
    {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);

        std::string list;
        for (size_t i = 0; i < effectiveCoreSet.cores.size(); ++i)
        {
            int core = effectiveCoreSet.cores[i];
            if (core < 0 || core >= CPU_SETSIZE)
            {
                pthread_attr_destroy(&attr);
                throw cIllegalArgExc("Invalid core index in CoreSet");
            }
            CPU_SET(core, &cpuset);

            if (!list.empty()) list.append(",");
            list.append(CerberusUtils::strPrint("%d", core));
        }

        int affRet = pthread_attr_setaffinity_np(&attr, sizeof(cpuset), &cpuset);

        if (affRet)
        {
            pthread_attr_destroy(&attr);
            throw cSystemExc("pthread_attr_setaffinity_np function failed: %s", strerror(affRet));
        }
    }

    void* rtStack      = nullptr;
    LSIZE rtStackBytes = 0;

    if (isRealtime)
    {
        size_t size = 0;

        if (stackSize > 0)
            size = static_cast<size_t>(stackSize);
        else
        {
            size_t attrSize = 0;
            int sizeRet     = pthread_attr_getstacksize(&attr, &attrSize);

            if (sizeRet)
            {
                pthread_attr_destroy(&attr);
                throw cSystemExc("pthread_attr_getstacksize function failed: %s", strerror(sizeRet));
            }

            size = attrSize;
        }

        const long page = sysconf(_SC_PAGESIZE);
        const size_t align = (page > 0) ? static_cast<size_t>(page) : 4096u;
        size = (size + align - 1) & ~(align - 1);

        int mmapFlags = MAP_PRIVATE | MAP_ANONYMOUS;
#ifdef MAP_STACK
        mmapFlags |= MAP_STACK;
#endif

        rtStack = mmap(nullptr, size, PROT_READ | PROT_WRITE, mmapFlags, -1, 0);

        if (rtStack == MAP_FAILED)
        {
            pthread_attr_destroy(&attr);
            throw cSystemExc("mmap function failed: %s", strerror(errno));
        }

        memset(rtStack, 0, size);  // touch the memory pages

        if (mlock(rtStack, size) != 0)
        {
            munmap(rtStack, size);
            pthread_attr_destroy(&attr);
            throw cSystemExc("mlock function failed: %s", strerror(errno));
        }

        int attrRet = pthread_attr_setstack(&attr, rtStack, size);

        if (attrRet)
        {
            munmap(rtStack, size);
            pthread_attr_destroy(&attr);
            throw cSystemExc("pthread_attr_setstack function failed: %s", strerror(attrRet));
        }

        rtStackBytes = size;
    }
    else if (stackSize > 0)
    {
        int attrRet = pthread_attr_setstacksize(&attr, stackSize);  // use kernel-provided stack

        if (attrRet)
        {
            pthread_attr_destroy(&attr);
            throw cSystemExc("pthread_attr_setstacksize function failed: %s", strerror(attrRet));
        }
    }

    auto ret = pthread_create(&m_pthread, &attr, &_staticThread, this);
    pthread_attr_destroy(&attr);

    if (ret)
    {
        if (rtStack)
            munmap(rtStack, rtStackBytes);

        throw cSystemExc("pthread_create function failed: %s", strerror(ret));
    }

    m_stack     = rtStack;
    m_stackSize = rtStackBytes;

    if (isRealtime)
    {
        sched_param param{};
        int prio = sched_get_priority_max(SCHED_FIFO);

        if (prio < 0)
            throw cSystemExc("sched_get_priority_max function failed: %s", strerror(errno));

        param.sched_priority = prio;

        int schedRet = pthread_setschedparam(m_pthread, SCHED_FIFO, &param);

        if (schedRet)
            throw cSystemExc("pthread_setschedparam function failed: %s", strerror(schedRet));
    }
}
//=============================================================================
int Thread::tick() { return m_tickCallback(next(), this); }
//=============================================================================
void Thread::setThreadName(const std::string& name)
{
#ifdef LINUX_SYSTEM
    if (!name.empty()) pthread_setname_np(m_pthread, CerberusUtils::truncStr(name, 15).c_str());
#else
    (void)name;
#endif
}
//=============================================================================
void Thread::setDefaultCoreSet(const CoreSet& coreSet) { s_defaultCoreSet = coreSet; }
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
void Thread::checkIn(const std::string& name)
{
    setThreadName(name);
    cerberus::core::Recordable::checkIn(name);
}
//=============================================================================
Thread::Thread(ThreadPeriodicity periodicity, const TimeFrame& time, LSIZE stackSize, const CoreSet& coreSet)
    : ThreadBase(periodicity),
      m_pthread(),
      m_retValue(0),
      m_tickCallback(&defaultTickCallback),
      m_warmUpCallback(&defaultWarmUpCallback),
      m_coolDownCallback(&defaultCoolDownCallback),
      m_stack(nullptr),
      m_stackSize(0)
{
    _construct(periodicity, time, stackSize, coreSet);
}
//=============================================================================
Thread::Thread(LSIZE stackSize, const CoreSet& coreSet)
    : ThreadBase(TP_Message),
      m_pthread(),
      m_retValue(0),
      m_tickCallback(&defaultTickCallback),
      m_warmUpCallback(&defaultWarmUpCallback),
      m_coolDownCallback(&defaultCoolDownCallback),
      m_stack(nullptr),
      m_stackSize(0)
{
    _construct(TP_Message, TimeFrame(), stackSize, coreSet);
}
//=============================================================================
Thread::Thread(ThreadPeriodicity periodicity, LSIZE stackSize, const CoreSet& coreSet)
    : ThreadBase(periodicity),
      m_pthread(),
      m_retValue(0),
      m_tickCallback(&defaultTickCallback),
      m_warmUpCallback(&defaultWarmUpCallback),
      m_coolDownCallback(&defaultCoolDownCallback),
      m_stack(nullptr),
      m_stackSize(0)
{
    _construct(periodicity, TimeFrame(), stackSize, coreSet);
}
//=============================================================================
Thread::~Thread()
{
    checkOut();
    if (m_stack)
    {
        munmap(m_stack, m_stackSize);
        m_stack = nullptr;
        m_stackSize = 0;
    }
}
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
