#ifndef RECIPIENT_H
#define RECIPIENT_H

// This interface class defines a message recipient, i.e. an object whose can receive messages.
// The Recipient class also syncronizes the queue accesses, and provides memory usage tracking

#include <list>

#include "../Cerberus_global.h"
#include "../types.h"
#include "./mutex.h"

namespace cerberus
{
    class CERBERUS_EXPORT Recipient
    {
       private:
        std::list<cerberus_message> m_queue;
        mutable Mutex m_mutex;
        SIZE m_queueBytes, m_queueWarningBytes;

        void _check() const;

       protected:
        // Returns the message in front of the queue and removes it
        cerberus_message next();

        // Returns the message in front of the queue using deep-copy.
        // The message will remain in the queue
        cerberus_message nextKeep() const;

        // Delete all the messages
        void clear();

        // Set the queue size warning in bytes.
        // When the threshold is reached, a warning log is printed in cerberus log channel
        void setQueueWarning(SIZE bytes);

        // Get the queue size in bytes
        SIZE getQueueBytesCount();

        // This method will be called by the Thread who calls addMessage()
        // and must be used just to signal a new message event (no processing on message here)
        // This method is guaranteed to be called with the mutex locked
        // This version will be called once for every addMessage() call
        virtual void newMsg();

        // This method will be called by the Thread who calls addMessage()
        // and must be used just to signal a new message event (no processing on message here)
        // This method is guaranteed to be called with the mutex locked
        // This version will be called only when the queue size transit from 0 to 1 messages
        virtual void newMsg_first();

       public:
        Recipient();

        // Adds a message at the end of the queue
        void addMessage(cerberus_message message);

        // Returns the size of the queue
        size_t size() const;

        // Tells wether the queue has at least one message
        bool hasMessage() const;
    };
}  // namespace cerberus

#endif  // RECIPIENT_H
