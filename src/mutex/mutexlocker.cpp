#include "mutexlocker.h"

using namespace cerberus::mutex;

//============================================================================
MutexLocker::MutexLocker(Mutex* mutex)
    : m_mutex(mutex)
{
    m_mutex->lock();
}
//============================================================================
MutexLocker::MutexLocker(Mutex& mutex)
    : m_mutex(&mutex)
{
    m_mutex->lock();
}
//============================================================================
MutexLocker::~MutexLocker() { m_mutex->unlock(); }
//============================================================================
