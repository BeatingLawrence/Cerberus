#include "mutex.h"

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

#include "src/cerberus.h"
#include "src/exception/exception.h"

using namespace crb;

//============================================================================
Mutex::Mutex(MutexType type)
    :
#ifdef WINDOWS_SYSTEM
      m_pmutex(nullptr),
      m_type(type),
#else
      m_pmutex(),
#endif
      m_valid(false)
{
#ifdef WINDOWS_SYSTEM
    switch (type)
    {
        case Simple:
        {
            auto lock = new SRWLOCK;
            InitializeSRWLock(lock);
            m_pmutex = lock;
        }
        break;

        case Recursive:
        {
            auto lock = new CRITICAL_SECTION;
            InitializeCriticalSection(lock);
            m_pmutex = lock;
        }
        break;
    }

    m_valid = true;
#else
    pthread_mutexattr_t attr{};

    if (pthread_mutexattr_init(&attr))
    {
        throw cSystemExc("pthread_mutexattr_init error");
    }

    int kind = 0;

    switch (type)
    {
        case Simple:
            kind = PTHREAD_MUTEX_ERRORCHECK;
            break;

        case Recursive:
            kind = PTHREAD_MUTEX_RECURSIVE;
            break;
    }

    if (pthread_mutexattr_settype(&attr, kind))
    {
        throw cSystemExc("pthread_mutexattr_settype error");
    }

    auto ret = pthread_mutex_init(&m_pmutex, &attr);  // default parameters

    if (ret)
    {
        throw cSystemExc("pthread_mutex_init error %s", strerror(ret));
    }

    pthread_mutexattr_destroy(&attr);

    m_valid = true;
#endif
}
//============================================================================
Mutex::Mutex(Mutex &&other)
    :
#ifdef WINDOWS_SYSTEM
      m_pmutex(other.m_pmutex),
      m_type(other.m_type),
#else
      m_pmutex(other.m_pmutex),
#endif
      m_valid(other.m_valid)
{
#ifdef WINDOWS_SYSTEM
    other.m_pmutex = nullptr;
#endif
    other.m_valid = false;
}
//============================================================================
Mutex::~Mutex()
{
    if (!m_valid) return;
#ifdef WINDOWS_SYSTEM
    if (m_type == Recursive)
    {
        DeleteCriticalSection(static_cast<CRITICAL_SECTION*>(m_pmutex));
        delete static_cast<CRITICAL_SECTION*>(m_pmutex);
    }
    else
    {
        delete static_cast<SRWLOCK*>(m_pmutex);
    }

    m_pmutex = nullptr;
#else
    pthread_mutex_unlock(&m_pmutex);
    pthread_mutex_destroy(&m_pmutex);
#endif
}
//============================================================================
bool Mutex::lock(bool block)
{
    if (!m_valid)
    {
        throw cIllegalStateExc("lock called on an invalid Mutex");
    }

#ifdef WINDOWS_SYSTEM
    if (m_type == Recursive)
    {
        if (block)
        {
            EnterCriticalSection(static_cast<CRITICAL_SECTION*>(m_pmutex));
            return true;
        }

        return TryEnterCriticalSection(static_cast<CRITICAL_SECTION*>(m_pmutex)) != 0;
    }

    if (block)
    {
        AcquireSRWLockExclusive(static_cast<SRWLOCK*>(m_pmutex));
        return true;
    }

    return TryAcquireSRWLockExclusive(static_cast<SRWLOCK*>(m_pmutex)) != 0;
#else
    int ret = 0;

    if (block)
        ret = pthread_mutex_lock(&m_pmutex);
    else
        ret = pthread_mutex_trylock(&m_pmutex);

    if (ret)
    {
        if (!block && ret == EBUSY)
        {
            return false;
        }
#ifdef LINUX_SYSTEM
        else if (ret == EOWNERDEAD)
        {
            if (pthread_mutex_consistent(&m_pmutex))
            {
                throw cSystemExc("pthread_mutex_consistent error %s", strerror(ret));
            }

            logWarning("Mutex has been recovered from inconsistent state");
            return true;
        }
#endif
        throw cSystemExc("pthread_mutex_%s error %s", block ? "lock" : "trylock", strerror(ret));
    }

    return true;
#endif
}
//=============================================================================
void Mutex::unlock()
{
    if (!m_valid)
    {
        throw cIllegalStateExc("unlock called on an invalid Mutex");
    }

#ifdef WINDOWS_SYSTEM
    if (m_type == Recursive)
        LeaveCriticalSection(static_cast<CRITICAL_SECTION*>(m_pmutex));
    else
        ReleaseSRWLockExclusive(static_cast<SRWLOCK*>(m_pmutex));
#else
    auto ret = pthread_mutex_unlock(&m_pmutex);

    if (ret)
    {
        throw cSystemExc("pthread_mutex_unlock error %s", strerror(ret));
    }
#endif
}
//=============================================================================
