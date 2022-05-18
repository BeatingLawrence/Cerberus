#ifndef CERBERUS_THREAD_THREADBASE_H
#define CERBERUS_THREAD_THREADBASE_H

#include "../mutex/mutex.h"
#include "../message/messagequeue.h"

namespace cerberus
{
    namespace thread
    {
        class CERBERUS_EXPORT ThreadBase
        {
            private:
                mutable mutex::Mutex m_mutex;

                message::MessageQueue m_queue;

                bool m_pausedFlag;

                bool m_terminateFlag;

            protected:
                ThreadBase();

                virtual ~ThreadBase();

                void setPausedFlag(bool state);

                void setTerminateFlag(bool state);

                bool getPausedFlag();

                bool getTerminateFlag() const;

                message::cerberus_message nextMessage();

                message::cerberus_message nextMessageKeep() const;

                bool isQueueEmpty() const;

            public:
                void addMessage(message::cerberus_message message);

                size_t messageCount() const;
        };
    }
}

#endif // CERBERUS_THREAD_THREADBASE_H
