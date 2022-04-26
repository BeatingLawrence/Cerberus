#ifndef CERBERUS_MUTEX_MUTEXLOCKER_H
#define CERBERUS_MUTEX_MUTEXLOCKER_H

/*  This class provides a convenient and secure way to lock and unlock mutexes.
 *
 *  When a MutexLocker is constructed, the corresponding mutex passed as
 *  argument is locked. When the MutexLocker is distructed afterwards, the
 *  associated mutex is unlocked.
 *
 *  In this way, the developer has not to remember to unlock a specific mutex.
 *  It will be automatically unlocked while the thread will reach the end of the calling method.
 */

#include "./mutex.h"
#include "../Cerberus_global.h"

namespace cerberus
{
    namespace mutex
    {
        class CERBERUS_EXPORT MutexLocker
        {
            private:
                Mutex* m_mutex;

            public:
                MutexLocker() = delete;

                MutexLocker(const MutexLocker& other) = delete;

                MutexLocker(Mutex* mutex);

                ~MutexLocker();
        };
    }
}

#endif // CERBERUS_MUTEX_MUTEXLOCKER_H
