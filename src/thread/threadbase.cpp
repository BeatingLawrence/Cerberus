#include "threadbase.h"
#include "../mutex/mutexlocker.h"

using namespace cerberus::thread;
using namespace cerberus::mutex;

//=============================================================================
ThreadBase::ThreadBase() :
    m_pausedFlag(true),
    m_terminateFlag(false)
{
}
//=============================================================================
void ThreadBase::setPausedFlag(bool state)
{
    MutexLocker locker(&m_mutex);
    m_pausedFlag = state;
}
//=============================================================================
void ThreadBase::setTerminateFlag(bool state)
{
    MutexLocker locker(&m_mutex);
    m_terminateFlag = state;
}
//=============================================================================
bool ThreadBase::getPausedFlag()
{
    MutexLocker locker(&m_mutex);
    return m_pausedFlag;
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
}
//=============================================================================
size_t ThreadBase::messageCount()
{
    MutexLocker locker(&m_mutex);
    return m_queue.size();
}
//=============================================================================
