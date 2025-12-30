#ifndef CERBERUS_THREAD_THREADBASE_H
#define CERBERUS_THREAD_THREADBASE_H

#include <pthread.h>

#include "../core/recordable.h"
#include "./recipient.h"
#include "mutex.h"

namespace cerberus
{
    class ThreadBase : public core::Recordable, public Recipient
    {
       private:
        mutable Mutex m_mutex;

        pthread_cond_t m_cond;

        bool m_pausedFlag, m_terminateFlag, m_dead, m_rescheduling;

        // Call this method with the mutex locked!
        void setPausedFlag(bool state);

        virtual void newMsg_first() override;

       protected:
        ThreadBase() = delete;

        ThreadBase(const ThreadBase &other) = delete;

        ThreadBase(ThreadBase &&other) = delete;

        ThreadBase(ThreadPeriodicity periodicity, const std::string &name);

        virtual ~ThreadBase();

        ThreadPeriodicity m_periodicity;

        void pause();

        bool getTerminateFlag() const;

        bool getPausedFlag() const;

        void dead();

        void reschedule();

        void resetRescheduling();

        bool isRescheduling();

        // stop only if the queue is empty
        void queueCheckStop();

       public:
        void start();

        void stop();

        void terminate();

        bool isDead();

        inline ThreadPeriodicity getPeriodicity() const { return m_periodicity; }

        using Recipient::size;
    };
}  // namespace cerberus

#endif  // CERBERUS_THREAD_THREADBASE_H
