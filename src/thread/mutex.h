#ifndef CERBERUS_MUTEX_MUTEX_H
#define CERBERUS_MUTEX_MUTEX_H

/*  This class represents a mutex.
 *
 *  Mutexes can be used to synchronize the
 *  memory accesses performed by threads.
 *
 *  This interface offers locking, unlocking and conditional-locking of mutexes.
 */

#include <pthread.h>

#include "../Cerberus_global.h"
#include "../types.h"

namespace crb
{
    class ThreadBase;

    class CERBERUS_EXPORT Mutex
    {
        friend class ::crb::ThreadBase;

       private:
        pthread_mutex_t m_pmutex;
        bool m_valid;

       public:
        Mutex(MutexType type = Simple);

        Mutex(const Mutex& other) = delete;

        Mutex(Mutex&& other);

        ~Mutex();

        // Takes mutex ownership. If block is true, this call will block or not,
        // depending on the state of the mutex and it will always return true.
        // If block is false and the mutex already locked, this call will not block and will return false,
        // avoiding mutex locking. If block is false and the mutex is lockable, this call will not block and
        // will return true, effectively locking the mutex. An excption will be thrown if the instance is
        // invalid
        bool lock(bool block = true);

        // Unlocks the mutex. Do not attempt to call this before lock().
        // An excption will be thrown if the instance is invalid
        void unlock();
    };
}  // namespace crb

#endif  // CERBERUS_MUTEX_MUTEX_H
