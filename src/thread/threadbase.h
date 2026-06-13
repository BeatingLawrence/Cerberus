#ifndef CERBERUS_THREAD_THREADBASE_H
#define CERBERUS_THREAD_THREADBASE_H

#ifndef WINDOWS_SYSTEM
#include <pthread.h>
#endif

#include "../core/recordable.h"
#include "./recipient.h"
#include "mutex.h"

namespace crb
{
    class ThreadBase : public core::Recordable, public Recipient
    {
       private:
        mutable Mutex m_mutex;

#ifdef WINDOWS_SYSTEM
        void* m_cond;
#else
        pthread_cond_t m_cond;
#endif

        bool m_pausedFlag, m_terminateFlag, m_dead, m_rescheduling;

        // Call this method with the mutex locked!
        void setPausedFlag(bool state);

        CERBERUS_EXPORT virtual void newMsg_first() override;

       protected:
        ThreadBase() = delete;

        ThreadBase(const ThreadBase &other) = delete;

        ThreadBase(ThreadBase &&other) = delete;

        CERBERUS_EXPORT ThreadBase(ThreadPeriodicity periodicity);

        CERBERUS_EXPORT virtual ~ThreadBase();

        ThreadPeriodicity m_periodicity;

        CERBERUS_EXPORT void pause();

        CERBERUS_EXPORT bool getTerminateFlag() const;

        CERBERUS_EXPORT bool getPausedFlag() const;

        CERBERUS_EXPORT void dead();

        CERBERUS_EXPORT void reschedule();

        CERBERUS_EXPORT void resetRescheduling();

        CERBERUS_EXPORT bool isRescheduling();

        // stop only if the queue is empty
        CERBERUS_EXPORT void queueCheckStop();

       public:
        CERBERUS_EXPORT void start();

        CERBERUS_EXPORT void stop();

        CERBERUS_EXPORT void terminate();

        CERBERUS_EXPORT bool isDead();

        inline ThreadPeriodicity getPeriodicity() const { return m_periodicity; }

        using Recipient::size;
    };
}  // namespace crb

#endif  // CERBERUS_THREAD_THREADBASE_H
