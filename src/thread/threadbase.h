#ifndef CERBERUS_THREAD_THREADBASE_H
#define CERBERUS_THREAD_THREADBASE_H

#include <pthread.h>

#include "../core/recordable.h"
#include "../message/messagequeue.h"
#include "mutex.h"

namespace cerberus
{
    class ThreadBase : public core::Recordable
    {
       private:
        mutable Mutex m_mutex;

        pthread_cond_t m_cond;

        MessageQueue m_queue;

        bool m_pausedFlag, m_terminateFlag, m_dead;

        // Call this method with the mutex locked!
        void setPausedFlag(bool state);

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

        cerberus_message nextMessage();

        cerberus_message nextMessageKeep() const;

        void discardMessageQueue();

        bool isQueueEmpty() const;

        void dead();

       public:
        void addMessage(cerberus_message message);

        size_t messageCount() const;

        void start();

        void stop();

        void terminate();

        bool isDead();

        inline ThreadPeriodicity getPeriodicity() const { return m_periodicity; };
    };
}  // namespace cerberus

#endif  // CERBERUS_THREAD_THREADBASE_H
