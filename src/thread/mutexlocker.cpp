#include "mutexlocker.h"

#include "src/thread/mutex.h"

using namespace crb;

//============================================================================
void MutexLocker::unref()
{
    if (!m_data) return;

    if (m_data->instances == 1)  // last instance is being destroyed
    {
        m_data->mutex->unlock();
        delete m_data;
    }
    else
        m_data->instances--;
}
//============================================================================
MutexLocker::MutexLocker()
    : m_data(nullptr)
{
}
//============================================================================
MutexLocker::MutexLocker(const MutexLocker& other)
    : m_data(other.m_data)
{
    if (m_data) m_data->instances++;
}
//============================================================================
MutexLocker::MutexLocker(Mutex* mutex)
    : m_data(new MutexLockerData({mutex, 1}))
{
    mutex->lock();
}
//============================================================================
MutexLocker::MutexLocker(Mutex& mutex)
    : m_data(new MutexLockerData({&mutex, 1}))
{
    mutex.lock();
}
//============================================================================
MutexLocker::~MutexLocker() { unref(); }
//============================================================================
void MutexLocker::operator=(const MutexLocker& other)
{
    if (this == &other) return;

    unref();

    m_data = other.m_data;

    if (m_data) m_data->instances++;
}
//============================================================================
bool MutexLocker::isValid() { return m_data != nullptr; }
//============================================================================
