#include "mutex.h"

#include <cstring>

#include "src/core/cerberuslog.h"
#include "src/exception/exceptioncatalog.h"

using namespace cerberus::mutex;

//============================================================================
Mutex::Mutex(MutexType type)
    : m_pmutex()
{
    pthread_mutexattr_t attr{};

    if (pthread_mutexattr_init(&attr))
    {
        throw cerberusSystemExc("pthread_mutexattr_init error");
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
        throw cerberusSystemExc("pthread_mutexattr_settype error");
    }

    auto ret = pthread_mutex_init(&m_pmutex, &attr);  // default parameters

    if (ret)
    {
        throw cerberusSystemExc("pthread_mutex_init error %s", strerror(ret));
    }

    pthread_mutexattr_destroy(&attr);
}
//============================================================================
Mutex::Mutex(const Mutex &other)
    : m_pmutex(other.m_pmutex)
{
    // noop
}
//============================================================================
Mutex::~Mutex()
{
    pthread_mutex_unlock(&m_pmutex);
    pthread_mutex_destroy(&m_pmutex);
}
//============================================================================
bool Mutex::lock(bool block)
{
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
                throw cerberusSystemExc("pthread_mutex_consistent error %s", strerror(ret));
            }

            logWarning("Mutex has been recovered from inconsistent state");
            return true;
        }
#endif
        throw cerberusSystemExc("pthread_mutex_%s error %s", block ? "lock" : "trylock", strerror(ret));
    }

    return true;
}
//=============================================================================
void Mutex::unlock()
{
    auto ret = pthread_mutex_unlock(&m_pmutex);

    if (ret)
    {
        throw cerberusSystemExc("pthread_mutex_unlock error %s", strerror(ret));
    }
}
//=============================================================================
