#include "threadbase.h"

#ifdef WINDOWS_SYSTEM
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <cstring>

#include "../exception/exception.h"
#include "../message/message.h"  // IWYU pragma: export
#include "mutexlocker.h"

using namespace crb;

//=============================================================================
ThreadBase::ThreadBase(ThreadPeriodicity periodicity)
    : Recordable(),
      Recipient(&m_mutex),
      m_mutex(),
      m_cond(),
      m_pausedFlag(true),
      m_terminateFlag(false),
      m_dead(false),
      m_rescheduling(false),
      m_periodicity(periodicity)
{
#ifdef WINDOWS_SYSTEM
    m_cond = new CONDITION_VARIABLE;
    InitializeConditionVariable(static_cast<CONDITION_VARIABLE*>(m_cond));
    return;
#else
    pthread_condattr_t attr{};

    int ret = pthread_condattr_init(&attr);

    if (ret) throw cSystemExc("pthread_condattr_init error %s", strerror(ret));

    ret = pthread_cond_init(&m_cond, &attr);

    if (ret) throw cSystemExc("pthread_cond_init error %s", strerror(ret));

    pthread_condattr_destroy(&attr);
#endif
}
//=============================================================================
ThreadBase::~ThreadBase()
{
#ifdef WINDOWS_SYSTEM
    delete static_cast<CONDITION_VARIABLE*>(m_cond);
    m_cond = nullptr;
#else
    pthread_cond_destroy(&m_cond);
#endif
}
//=============================================================================
void ThreadBase::setPausedFlag(bool state)
{
    m_pausedFlag = state;

#ifdef WINDOWS_SYSTEM
    WakeConditionVariable(static_cast<CONDITION_VARIABLE*>(m_cond));
#else
    int ret = pthread_cond_signal(&m_cond);

    if (ret) throw cSystemExc("pthread_cond_signal error %s", strerror(ret));
#endif
}
//=============================================================================
void ThreadBase::newMsg_first()
{
    MutexLocker locker(&m_mutex);
    if (m_periodicity == TP_Message && m_pausedFlag) setPausedFlag(false);
}
//=============================================================================
void ThreadBase::pause()
{
    MutexLocker locker(&m_mutex);

    if (m_terminateFlag) return;  // skip pause if the termination is requested

    if (m_rescheduling) return;  // skip pause if rescheduling has been requested

    while (m_pausedFlag)
    {
#ifdef WINDOWS_SYSTEM
        BOOL ret = 0;
        if (m_mutex.m_type == Recursive)
        {
            ret = SleepConditionVariableCS(static_cast<CONDITION_VARIABLE*>(m_cond),
                                           static_cast<CRITICAL_SECTION*>(m_mutex.m_pmutex), INFINITE);
        }
        else
        {
            ret = SleepConditionVariableSRW(static_cast<CONDITION_VARIABLE*>(m_cond),
                                            static_cast<SRWLOCK*>(m_mutex.m_pmutex), INFINITE, 0);
        }

        if (!ret) throw cSystemExc("SleepConditionVariable error %lu", static_cast<unsigned long>(GetLastError()));
#else
        int ret = pthread_cond_wait(&m_cond, &m_mutex.m_pmutex);  // this call internally unlocks the mutex

        if (ret) throw cSystemExc("pthread_cond_wait error %s", strerror(ret));
#endif
    }
}
//=============================================================================
bool ThreadBase::getTerminateFlag() const
{
    MutexLocker locker(&m_mutex);
    return m_terminateFlag;
}
//=============================================================================
bool ThreadBase::getPausedFlag() const
{
    MutexLocker locker(&m_mutex);
    return m_pausedFlag;
}
//=============================================================================
void ThreadBase::dead()
{
    MutexLocker locker(&m_mutex);
    m_dead = true;
}
//=============================================================================
void ThreadBase::reschedule()
{
    MutexLocker locker(&m_mutex);
    m_rescheduling = true;
}
//=============================================================================
void ThreadBase::resetRescheduling()
{
    MutexLocker locker(&m_mutex);
    m_rescheduling = false;
}
//=============================================================================
bool ThreadBase::isRescheduling()
{
    MutexLocker locker(&m_mutex);
    return m_rescheduling;
}
//=============================================================================
void ThreadBase::queueCheckStop()
{
    MutexLocker locker(&m_mutex);
    if (!size_nomutex()) setPausedFlag(true);
}
//=============================================================================
void ThreadBase::start()
{
    MutexLocker locker(&m_mutex);
    setPausedFlag(false);
}
//=============================================================================
void ThreadBase::stop()
{
    MutexLocker locker(&m_mutex);
    setPausedFlag(true);
}
//=============================================================================
void ThreadBase::terminate()
{
    MutexLocker locker(&m_mutex);
    m_terminateFlag = true;
}
//=============================================================================
bool ThreadBase::isDead()
{
    MutexLocker locker(&m_mutex);
    return m_dead;
}
//=============================================================================
