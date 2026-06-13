#include "thread.h"

#ifdef WINDOWS_SYSTEM
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <process.h>
#include <windows.h>
#else
#include <limits.h>
#include <pthread.h>
#include <sched.h>
#include <sys/mman.h>
#include <unistd.h>
#endif

#include <cerrno>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>

#include "../cerberus.h"
#include "../core/signalhandler.h"
#include "../core/cerberusutils.h"
#include "../exception/exception.h"

using namespace crb;

CoreSet Thread::s_defaultCoreSet;

#ifdef WINDOWS_SYSTEM
namespace
{
std::string windowsErrorString(DWORD error)
{
    char* message = nullptr;
    DWORD len = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                                   FORMAT_MESSAGE_IGNORE_INSERTS,
                               nullptr, error, 0, reinterpret_cast<LPSTR>(&message), 0, nullptr);

    std::string ret = CerberusUtils::strPrint("Windows error %lu", static_cast<unsigned long>(error));
    if (len != 0 && message != nullptr)
    {
        ret.append(": ");
        ret.append(message, len);
        LocalFree(message);
    }

    return ret;
}

void closeThreadHandle(void*& handle)
{
    if (!handle) return;
    CloseHandle(static_cast<HANDLE>(handle));
    handle = nullptr;
}

void terminateCreatedThread(void*& handle)
{
    if (!handle) return;
    TerminateThread(static_cast<HANDLE>(handle), 1);
    closeThreadHandle(handle);
}

void setWindowsThreadName(HANDLE handle, const std::string& name)
{
    if (!handle || name.empty()) return;

    using SetThreadDescriptionFn = HRESULT(WINAPI*)(HANDLE, PCWSTR);
    HMODULE kernel = GetModuleHandleA("Kernel32.dll");
    if (!kernel) return;

    auto setThreadDescription = reinterpret_cast<SetThreadDescriptionFn>(
        GetProcAddress(kernel, "SetThreadDescription"));
    if (!setThreadDescription) return;

    std::wstring wideName(name.begin(), name.end());
    setThreadDescription(handle, wideName.c_str());
}

GROUP_AFFINITY coreSetToWindowsGroupAffinity(const CoreSet& coreSet)
{
    GROUP_AFFINITY affinity{};
    bool groupSelected = false;
    constexpr int maskBits = static_cast<int>(std::numeric_limits<KAFFINITY>::digits);
    const WORD groupCount = GetActiveProcessorGroupCount();

    for (int core : coreSet.cores)
    {
        if (core < 0)
        {
            throw cIllegalArgExc("Invalid core index in CoreSet");
        }

        DWORD localCore = static_cast<DWORD>(core);
        WORD targetGroup = 0;
        bool found = false;

        for (WORD group = 0; group < groupCount; ++group)
        {
            DWORD groupCores = GetActiveProcessorCount(group);
            if (localCore < groupCores)
            {
                targetGroup = group;
                found = true;
                break;
            }

            localCore -= groupCores;
        }

        if (!found || localCore >= static_cast<DWORD>(maskBits))
        {
            throw cIllegalArgExc("Invalid core index in CoreSet");
        }

        if (!groupSelected)
        {
            affinity.Group = targetGroup;
            groupSelected = true;
        }
        else if (affinity.Group != targetGroup)
        {
            throw cIllegalArgExc("CoreSet spans multiple Windows processor groups");
        }

        affinity.Mask |= (static_cast<KAFFINITY>(1) << localCore);
    }

    return affinity;
}
}  // namespace
#endif

//=============================================================================
#ifdef WINDOWS_SYSTEM
unsigned __stdcall Thread::_staticThread(void* context)
#else
void* Thread::_staticThread(void* context)
#endif
{
    ((Thread*)context)->_thread();
#ifdef WINDOWS_SYSTEM
    return 0;
#else
    return nullptr;
#endif
}
//=============================================================================
void Thread::_thread()
{
#ifndef WINDOWS_SYSTEM
    crb::core::maskTerminationSignalsForCurrentThread();
#endif

    // set system thread name if supported
#if defined(WINDOWS_SYSTEM)
    setWindowsThreadName(GetCurrentThread(), m_threadName);
#elif defined(APPLE_SYSTEM)
    if (!m_threadName.empty()) pthread_setname_np(CerberusUtils::truncStr(m_threadName, 63).c_str());
#elif defined(LINUX_SYSTEM)
    if (!m_threadName.empty())
        pthread_setname_np(pthread_self(), CerberusUtils::truncStr(m_threadName, 15).c_str());
#endif

    bool firstRun = true;

    while (true)
    {
        pause();

        const bool wasRescheduled = isRescheduling();
        resetRescheduling();

        while (true)
        {
            auto terminationMsg = next(TERMINATION_MSG_QUEUE);
            if (!terminationMsg) break;
            if (terminationMsg->id() == CRB_MESSAGE_TERM_ID)
            {
                terminate();
                break;
            }
        }

        if (getTerminateFlag()) break;

        if (firstRun)
        {
            warmUp();
            firstRun = false;
        }

        if (m_periodicity == TP_Periodic || m_periodicity == TP_Periodic_realtime ||
            m_periodicity == TP_PeriodicMessage)
        {
            m_periodTimer.startDeadline();
        }

        switch (m_periodicity)
        {
            case TP_Message:
            {
                if (wasRescheduled || hasMessage()) m_retValue = tick();
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
            case TP_Continuos_realtime:
            {
                m_retValue = tick();
            }
            break;

            case TP_Trigger:
            {
                m_retValue = tick();
                if (!isRescheduling()) stop();
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
    m_overrun = m_periodTimer.waitDeadline();
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
        m_periodTimer.setTime(time);
    }

#ifdef WINDOWS_SYSTEM
    const bool isRealtime = (periodicity == ThreadPeriodicity::TP_Periodic_realtime ||
                             periodicity == ThreadPeriodicity::TP_Continuos_realtime);

    if (stackSize > static_cast<LSIZE>(std::numeric_limits<unsigned>::max()))
    {
        throw cIllegalArgExc("Invalid stack size: too large for Windows thread creation");
    }

    CoreSet effectiveCoreSet = coreSet;
    if (effectiveCoreSet.empty() && !s_defaultCoreSet.empty())
    {
        effectiveCoreSet = s_defaultCoreSet;
    }

    unsigned threadId = 0;
    uintptr_t handle = _beginthreadex(nullptr, static_cast<unsigned>(stackSize), &_staticThread, this,
                                      CREATE_SUSPENDED, &threadId);

    if (handle == 0)
    {
        throw cSystemExc("_beginthreadex function failed: %s", strerror(errno));
    }

    m_threadHandle = reinterpret_cast<void*>(handle);
    m_threadId     = threadId;

    if (!effectiveCoreSet.empty())
    {
        GROUP_AFFINITY affinity = coreSetToWindowsGroupAffinity(effectiveCoreSet);
        if (!SetThreadGroupAffinity(static_cast<HANDLE>(m_threadHandle), &affinity, nullptr))
        {
            DWORD err = GetLastError();
            terminateCreatedThread(m_threadHandle);
            throw cSystemExc("SetThreadGroupAffinity function failed: %s", windowsErrorString(err).c_str());
        }
    }

    if (isRealtime)
    {
        if (!SetThreadPriority(static_cast<HANDLE>(m_threadHandle), THREAD_PRIORITY_TIME_CRITICAL))
        {
            DWORD err = GetLastError();
            terminateCreatedThread(m_threadHandle);
            throw cSystemExc("SetThreadPriority function failed: %s", windowsErrorString(err).c_str());
        }
    }

    setWindowsThreadName(static_cast<HANDLE>(m_threadHandle), m_threadName);

    if (ResumeThread(static_cast<HANDLE>(m_threadHandle)) == static_cast<DWORD>(-1))
    {
        DWORD err = GetLastError();
        terminateCreatedThread(m_threadHandle);
        throw cSystemExc("ResumeThread function failed: %s", windowsErrorString(err).c_str());
    }
#else
    pthread_attr_t attr{};

    if (pthread_attr_init(&attr))  // default attributes
        throw cSystemExc("pthread_attr_init function failed");

    const bool isRealtime = (periodicity == ThreadPeriodicity::TP_Periodic_realtime ||
                             periodicity == ThreadPeriodicity::TP_Continuos_realtime);

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

#ifndef APPLE_SYSTEM
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
#endif

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

        const long page    = sysconf(_SC_PAGESIZE);
        const size_t align = (page > 0) ? static_cast<size_t>(page) : 4096u;
        size               = (size + align - 1) & ~(align - 1);

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
        if (rtStack) munmap(rtStack, rtStackBytes);

        throw cSystemExc("pthread_create function failed: %s", strerror(ret));
    }

    m_stack     = rtStack;
    m_stackSize = rtStackBytes;

    if (isRealtime)
    {
        sched_param param{};
        int prio = sched_get_priority_max(SCHED_FIFO);

        if (prio < 0) throw cSystemExc("sched_get_priority_max function failed: %s", strerror(errno));

        param.sched_priority = prio;

        int schedRet = pthread_setschedparam(m_pthread, SCHED_FIFO, &param);

        if (schedRet) throw cSystemExc("pthread_setschedparam function failed: %s", strerror(schedRet));
    }
#endif
}
//=============================================================================
int Thread::tick() { return m_tickCallback(next(), this); }
//=============================================================================
void Thread::setThreadName(const std::string& name)
{
    m_threadName = name;
#ifdef WINDOWS_SYSTEM
    setWindowsThreadName(static_cast<HANDLE>(m_threadHandle), m_threadName);
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
#ifdef WINDOWS_SYSTEM
    uint64_t milliseconds = static_cast<uint64_t>(splitted.seconds) * 1000u +
                            static_cast<uint64_t>((splitted.nanoseconds + 999999u) / 1000000u);

    while (milliseconds > 0)
    {
        constexpr DWORD maxSleepChunk = INFINITE - 1u;
        DWORD chunk = milliseconds > static_cast<uint64_t>(maxSleepChunk) ? maxSleepChunk
                                                                          : static_cast<DWORD>(milliseconds);
        Sleep(chunk);
        milliseconds -= chunk;
    }
#else
    timespec t{};
    t.tv_nsec = splitted.nanoseconds;
    t.tv_sec  = splitted.seconds;
    nanosleep(&t, NULL);
#endif
}
//=============================================================================
void Thread::checkIn(const std::string& name)
{
    setThreadName(name);
    crb::core::Recordable::checkIn(name);
}
//=============================================================================
Thread::Thread(ThreadPeriodicity periodicity, const TimeFrame& time, LSIZE stackSize, const CoreSet& coreSet)
    : ThreadBase(periodicity),
#ifdef WINDOWS_SYSTEM
      m_threadHandle(nullptr),
      m_threadId(0),
#else
      m_pthread(),
#endif
      m_time{},
      m_periodTimer(),
      m_overrun(false),
      m_stack(nullptr),
      m_stackSize(0),
      m_retValue(0),
      m_tickCallback(&defaultTickCallback),
      m_warmUpCallback(&defaultWarmUpCallback),
      m_coolDownCallback(&defaultCoolDownCallback)
{
#ifndef WINDOWS_SYSTEM
    crb::core::maskTerminationSignalsForCurrentThread();
#endif
    _construct(periodicity, time, stackSize, coreSet);
}
//=============================================================================
Thread::Thread(LSIZE stackSize, const CoreSet& coreSet)
    : ThreadBase(TP_Message),
#ifdef WINDOWS_SYSTEM
      m_threadHandle(nullptr),
      m_threadId(0),
#else
      m_pthread(),
#endif
      m_time{},
      m_periodTimer(),
      m_overrun(false),
      m_stack(nullptr),
      m_stackSize(0),
      m_retValue(0),
      m_tickCallback(&defaultTickCallback),
      m_warmUpCallback(&defaultWarmUpCallback),
      m_coolDownCallback(&defaultCoolDownCallback)
{
#ifndef WINDOWS_SYSTEM
    crb::core::maskTerminationSignalsForCurrentThread();
#endif
    _construct(TP_Message, TimeFrame(), stackSize, coreSet);
}
//=============================================================================
Thread::Thread(ThreadPeriodicity periodicity, LSIZE stackSize, const CoreSet& coreSet)
    : ThreadBase(periodicity),
#ifdef WINDOWS_SYSTEM
      m_threadHandle(nullptr),
      m_threadId(0),
#else
      m_pthread(),
#endif
      m_time{},
      m_periodTimer(),
      m_overrun(false),
      m_stack(nullptr),
      m_stackSize(0),
      m_retValue(0),
      m_tickCallback(&defaultTickCallback),
      m_warmUpCallback(&defaultWarmUpCallback),
      m_coolDownCallback(&defaultCoolDownCallback)
{
#ifndef WINDOWS_SYSTEM
    crb::core::maskTerminationSignalsForCurrentThread();
#endif
    _construct(periodicity, TimeFrame(), stackSize, coreSet);
}
//=============================================================================
Thread::~Thread()
{
    checkOut();
#ifdef WINDOWS_SYSTEM
    closeThreadHandle(m_threadHandle);
    m_threadId = 0;
#else
    if (m_stack)
    {
        munmap(m_stack, m_stackSize);
        m_stack     = nullptr;
        m_stackSize = 0;
    }
#endif
}
//=============================================================================
SplittedTime Thread::getTime() const { return m_time; }
//=============================================================================
bool Thread::isOverrun() const { return m_overrun; }
//=============================================================================
IntOpRes Thread::join(bool stop)
{
    if (stop)
    {
        terminate();  // set termination flag
        start();      // force the thread to run
    }

#ifdef WINDOWS_SYSTEM
    if (!m_threadHandle)
    {
        if (isDead()) return m_retValue;
        return OR_ThreadNotJoinable;
    }

    DWORD ret = WaitForSingleObject(static_cast<HANDLE>(m_threadHandle), INFINITE);
    if (ret == WAIT_FAILED)
    {
        IntOpRes toReturn(OR_Failure);
        toReturn.reason = windowsErrorString(GetLastError());
        return toReturn;
    }

    closeThreadHandle(m_threadHandle);
    m_threadId = 0;
    return m_retValue;
#else
    if (isDead()) return m_retValue;

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
#endif
}
//=============================================================================
crb::OpRes Thread::detach()
{
#ifdef WINDOWS_SYSTEM
    if (!m_threadHandle) return OR_ThreadNotJoinable;
    closeThreadHandle(m_threadHandle);
    m_threadId = 0;
    return OR_OK;
#else
    int ret = pthread_detach(m_pthread);

    if (ret)
    {
        OpRes toReturn(OR_Failure);
        toReturn.reason = strerror(ret);
        return toReturn;
    }

    return OR_OK;
#endif
}
//=============================================================================
void Thread::provideTickCallback(threadTickCallback callback) { m_tickCallback = callback; }
//=============================================================================
void Thread::provideWarmUpCallback(threadCallback callback) { m_warmUpCallback = callback; }
//=============================================================================
void Thread::provideCoolDownCallback(threadCallback callback) { m_coolDownCallback = callback; }
//=============================================================================
