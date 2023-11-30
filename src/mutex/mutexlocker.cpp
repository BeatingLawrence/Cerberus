#include "mutexlocker.h"

#include "src/mutex/mutex.h"

using namespace cerberus::mutex;

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
MutexLocker::~MutexLocker()
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
void MutexLocker::operator=(const MutexLocker& other)
{
    if (this == &other) return;

    if (m_data)
    {
        if (m_data->instances == 1)  // last instance is being destroyed
        {
            m_data->mutex->unlock();
            delete m_data;
        }
        else
            m_data->instances--;
    }

    m_data = other.m_data;

    if (m_data) m_data->instances++;
}
//============================================================================
bool MutexLocker::isValid() { return m_data != nullptr; }
//============================================================================
