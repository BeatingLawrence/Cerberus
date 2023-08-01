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

           protected:
            ThreadBase();

            virtual ~ThreadBase();

            void setPausedFlag(bool state);

            void setTerminateFlag(bool state);

            bool getPausedFlag();

            void pause();

            bool getTerminateFlag() const;

            message::cerberus_message nextMessage();

            message::cerberus_message nextMessageKeep() const;

            bool isQueueEmpty() const;

           public:
            void addMessage(message::cerberus_message message);

            size_t messageCount() const;
        };
    }  // namespace thread
}  // namespace cerberus

#endif  // CERBERUS_THREAD_THREADBASE_H
