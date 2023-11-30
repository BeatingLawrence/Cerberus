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

#include "../Cerberus_global.h"

namespace cerberus
{
    namespace mutex
    {
        class Mutex;
        class CERBERUS_EXPORT MutexLocker
        {
           private:
            struct MutexLockerData
            {
                Mutex* mutex;
                int instances;
            };

            MutexLockerData* m_data;

           public:
            MutexLocker();

            MutexLocker(const MutexLocker& other);

            MutexLocker(Mutex* mutex);

            MutexLocker(Mutex& mutex);

            ~MutexLocker();

            void operator=(const MutexLocker& other);

            bool isValid();
        };
    }  // namespace mutex
}  // namespace cerberus

#endif  // CERBERUS_MUTEX_MUTEXLOCKER_H
