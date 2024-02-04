#ifndef CERBERUS_THREAD_THREADBASE_H
#define CERBERUS_THREAD_THREADBASE_H

#include <pthread.h>

#include "../message/messagequeue.h"
#include "../mutex/mutex.h"

namespace cerberus
{
    namespace thread
    {
        class ThreadBase
        {
           private:
            mutable mutex::Mutex m_mutex;

            pthread_cond_t m_cond;

            message::MessageQueue m_queue;

            bool m_pausedFlag;

            bool m_terminateFlag;

            // Call this method with the mutex locked!
            void setPausedFlag(bool state);

           protected:
            ThreadBase(ThreadPeriodicity periodicity);

            virtual ~ThreadBase();

            ThreadPeriodicity m_periodicity;

            void pause();

            bool getTerminateFlag() const;

            cerberus_message nextMessage();

            cerberus_message nextMessageKeep() const;

            void discardMessageQueue();

            bool isQueueEmpty() const;

           public:
            void addMessage(cerberus_message message);

            size_t messageCount() const;

            void start();

            void stop();

            void terminate();

            inline ThreadPeriodicity getPeriodicity() const { return m_periodicity; };
        };
    }  // namespace thread
}  // namespace cerberus

#endif  // CERBERUS_THREAD_THREADBASE_H
