#include "threadbase.h"

#include <cstring>

#include "../exception/exception.h"
#include "../message/message.h"  // IWYU pragma: export
#include "mutexlocker.h"

using namespace cerberus;

//=============================================================================
ThreadBase::ThreadBase(ThreadPeriodicity periodicity, const std::string &name)
    : Recordable(Recordable::ObjectType::COBJ_Thread, name),
      m_mutex(),
      m_cond(),
      m_pausedFlag(true),
      m_terminateFlag(false),
      m_dead(false),
      m_periodicity(periodicity)
{
    pthread_condattr_t attr{};

    int ret = pthread_condattr_init(&attr);

    if (ret)
    {
        throw cSystemExc("pthread_condattr_init error %s", strerror(ret));
    }

    ret = pthread_cond_init(&m_cond, &attr);

    if (ret)
    {
        throw cSystemExc("pthread_cond_init error %s", strerror(ret));
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
        throw cSystemExc("pthread_cond_signal error %s", strerror(ret));
    }
}
//=============================================================================
void ThreadBase::pause()
{
    MutexLocker locker(&m_mutex);

    if (m_terminateFlag) return;  // skip pause if the termination is requested

    while (m_pausedFlag)
    {
        int ret = pthread_cond_wait(&m_cond, &m_mutex.m_pmutex);  // this call internally unlocks the mutex

        if (ret)
        {
            throw cSystemExc("pthread_cond_wait error %s", strerror(ret));
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
bool ThreadBase::getPausedFlag() const
{
    MutexLocker locker(&m_mutex);
    return m_pausedFlag;
}
//=============================================================================
cerberus::cerberus_message ThreadBase::nextMessage()
{
    MutexLocker locker(&m_mutex);

    if (m_queue.isEmpty()) return Message::create();  // invalid

    if (m_queue.size() == 1 && m_periodicity == TP_Message && !m_pausedFlag) setPausedFlag(true);

    return m_queue.next();
}
//=============================================================================
cerberus::cerberus_message ThreadBase::nextMessageKeep() const
{
    MutexLocker locker(&m_mutex);
    return m_queue.nextKeep();
}
//=============================================================================
void ThreadBase::discardMessageQueue()
{
    MutexLocker locker(&m_mutex);
    m_queue.clear();
}
//=============================================================================
bool ThreadBase::isQueueEmpty() const
{
    MutexLocker locker(&m_mutex);
    return m_queue.isEmpty();
}
//=============================================================================
void ThreadBase::dead()
{
    MutexLocker locker(&m_mutex);
    m_dead = true;
}
//=============================================================================
void ThreadBase::addMessage(cerberus_message message)
{
    MutexLocker locker(&m_mutex);
    m_queue.add(message);

    if (m_periodicity == TP_Message && m_pausedFlag)
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
bool ThreadBase::isDead()
{
    MutexLocker locker(&m_mutex);
    return m_dead;
}
//=============================================================================
