#include "threadbase.h"

#include <cstring>

#include "../mutex/mutexlocker.h"
#include "src/core/cerberuslog.h"

using namespace cerberus::thread;
using namespace cerberus::mutex;

//=============================================================================
ThreadBase::ThreadBase(ThreadPeriodicity periodicity)
    : m_mutex(),
      m_cond(),
      m_pausedFlag(true),
      m_terminateFlag(false),
      m_periodicity(periodicity)
{
    pthread_condattr_t attr{};

    int ret = pthread_condattr_init(&attr);

    if (ret)
    {
        throw cerberusSystemExc("pthread_condattr_init error %s", strerror(ret));
    }

    ret = pthread_cond_init(&m_cond, &attr);

    if (ret)
    {
        throw cerberusSystemExc("pthread_cond_init error %s", strerror(ret));
    }

    pthread_condattr_destroy(&attr);
}
//=============================================================================
ThreadBase::~ThreadBase() { pthread_cond_destroy(&m_cond); }
//=============================================================================
void ThreadBase::setPausedFlag(bool state)
{
    m_pausedFlag = state;

    int ret = pthread_cond_signal(&m_cond);

    if (ret)
    {
        throw cerberusSystemExc("pthread_cond_signal error %s", strerror(ret));
    }
}
//=============================================================================
void ThreadBase::pause()
{
    MutexLocker locker(&m_mutex);

    while (m_pausedFlag)
    {
        int ret = pthread_cond_wait(&m_cond, &m_mutex.m_pmutex);  // this call internally unlocks the mutex

        if (ret)
        {
            throw cerberusSystemExc("pthread_cond_wait error %s", strerror(ret));
        }
    }
}
//=============================================================================
bool ThreadBase::getTerminateFlag() const
{
    MutexLocker locker(&m_mutex);
    return m_terminateFlag;
}
//=============================================================================
cerberus::message::cerberus_message ThreadBase::nextMessage()
{
    MutexLocker locker(&m_mutex);

    if (m_queue.size() == 1 && m_periodicity == TP_NonPeriodic && !m_pausedFlag)
    {
        setPausedFlag(true);
    }

    return m_queue.next();
}
//=============================================================================
cerberus::message::cerberus_message ThreadBase::nextMessageKeep() const
{
    MutexLocker locker(&m_mutex);
    return m_queue.nextKeep();
}
//=============================================================================
bool ThreadBase::isQueueEmpty() const
{
    MutexLocker locker(&m_mutex);
    return m_queue.isEmpty();
}
//=============================================================================
void ThreadBase::addMessage(message::cerberus_message message)
{
    MutexLocker locker(&m_mutex);
    m_queue.add(message);

    if (m_periodicity == TP_NonPeriodic && m_pausedFlag)
    {
        setPausedFlag(false);
    }
}
//=============================================================================
size_t ThreadBase::messageCount() const
{
    MutexLocker locker(&m_mutex);
    return m_queue.size();
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
