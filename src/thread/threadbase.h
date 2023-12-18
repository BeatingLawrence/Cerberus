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
           public:
            enum ThreadPeriodicity
            {
                TP_NonPeriodic,
                TP_Periodic,
                TP_PeriodicQueue,
                TP_OneShot,
            };

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

            message::cerberus_message nextMessage();

            message::cerberus_message nextMessageKeep() const;

            void discardMessageQueue();

            bool isQueueEmpty() const;

           public:
            void addMessage(message::cerberus_message message);

            size_t messageCount() const;

            void start();

            void stop();

            void terminate();
        };
    }  // namespace thread
}  // namespace cerberus

#endif  // CERBERUS_THREAD_THREADBASE_H
