#ifndef CERBERUS_MUTEX_MUTEX_H
#define CERBERUS_MUTEX_MUTEX_H

/*  This class represents a mutex.
 *
 *  Mutexes can be used to synchronize the
 *  memory accesses performed by threads.
 *
 *  This interface offers locking, unlocking and conditional-locking of mutexes.
 */

#if !defined(WINDOWS_SYSTEM) && !defined(_WIN32) && !defined(WIN32)
#include <pthread.h>
#endif

#include "../Cerberus_global.h"
#include "../types.h"

namespace crb
{
    class ThreadBase;

    class Mutex
    {
        friend class ::crb::ThreadBase;

       private:
#if defined(WINDOWS_SYSTEM) || defined(_WIN32) || defined(WIN32)
        void* m_pmutex;
        MutexType m_type;
#else
        pthread_mutex_t m_pmutex;
#endif
        bool m_valid;

       public:
        CERBERUS_EXPORT Mutex(MutexType type = Simple);

        Mutex(const Mutex& other) = delete;

        CERBERUS_EXPORT Mutex(Mutex&& other);

        CERBERUS_EXPORT ~Mutex();

        // Takes mutex ownership. If block is true, this call will block or not,
        // depending on the state of the mutex and it will always return true.
        // If block is false and the mutex already locked, this call will not block and will return false,
        // avoiding mutex locking. If block is false and the mutex is lockable, this call will not block and
        // will return true, effectively locking the mutex. An excption will be thrown if the instance is
        // invalid
        CERBERUS_EXPORT bool lock(bool block = true);

        // Unlocks the mutex. Do not attempt to call this before lock().
        // An excption will be thrown if the instance is invalid
        CERBERUS_EXPORT void unlock();
    };
}  // namespace crb

#endif  // CERBERUS_MUTEX_MUTEX_H
