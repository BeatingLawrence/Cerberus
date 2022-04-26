#ifndef CERBERUS_MUTEX_MUTEX_H
#define CERBERUS_MUTEX_MUTEX_H

/*  This class represents a mutex.
 *
 *  Mutexes can be used to synchronize the
 *  memory accesses done by threads.
 *
 *  This implementation offers locking, unlocking and conditional-locking of mutexes.
 */

#include <mutex>
#include "../Cerberus_global.h"

namespace cerberus
{
    namespace mutex
    {
        class CERBERUS_EXPORT Mutex
        {
            private:
                std::mutex m_mutex;

            public:
                Mutex() = default;

                Mutex(const Mutex& other) = delete;

                //Takes mutex ownership. If block is true, this call will block or not,
                //depending on the state of the mutex and it will always return true.
                //If block is false and the mutex already locked, this call will not block and will return false, avoiding mutex locking.
                //If block is false and the mutex is lockable, this call will not block and will return true, effectively locking the mutex.
                bool lock(bool block = true);

                //Unlocks the mutex. Do not attempt to call this before lock().
                void unlock();
        };
    }
}

#endif // CERBERUS_MUTEX_MUTEX_H
