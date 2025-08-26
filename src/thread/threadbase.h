#ifndef CERBERUS_THREAD_THREADBASE_H
#define CERBERUS_THREAD_THREADBASE_H

#include <pthread.h>

#include "../core/recordable.h"
#include "mutex.h"

namespace cerberus
{
    class ThreadBase : public core::Recordable
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

        ThreadBase(const ThreadBase& other) = delete;

        ThreadBase(ThreadBase&& other) = delete;

        ThreadBase(ThreadPeriodicity periodicity, const std::string& name);

        virtual ~ThreadBase();

        ThreadPeriodicity m_periodicity;

        void pause();

        bool getTerminateFlag() const;

        bool getPausedFlag() const;

        void dead();

        void reschedule();

        void resetRescheduling();

        bool isRescheduling();

       public:
        void start();

        void stop();

        void terminate();

        bool isDead();

        inline ThreadPeriodicity getPeriodicity() const { return m_periodicity; };
    };
}  // namespace cerberus

#endif  // CERBERUS_THREAD_THREADBASE_H
