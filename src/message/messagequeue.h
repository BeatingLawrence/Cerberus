#ifndef CERBERUS_MESSAGE_MESSAGEQUEUE_H
#define CERBERUS_MESSAGE_MESSAGEQUEUE_H

/*  This class provides a message queue implementation.
 *
 *  The queue consists in a list of pointers to messages.
 *
 *  Respecting the behavoir of a queue, it is only
 *  possible to insert elements at the end of the queue and
 *  extract (and read) them from the beginning.
 *
 *  It is also possible to access to the first element read-only, without extracting.
 *
 *  Attempting to read the first element with the queue empty will throw an exception.
 *  Please verify the queue size using isEmpty() or size() methods before accessing elements.
 */

#include <list>
#include <memory>
#include "./message.h"

namespace cerberus
{
    namespace message
    {
        typedef std::shared_ptr<class MessageQueue> cerberus_messageQueue;

        class MessageQueue
        {
            private:
                std::list<cerberus_message> m_queue;

            public:
                static cerberus_messageQueue create();

                static cerberus_messageQueue create(const MessageQueue& other);

                MessageQueue();

                MessageQueue(const MessageQueue& other);

                //Adds a message at the end of the queue
                void add(cerberus_message message);

                //Returns the first message in the queue and removes it
                cerberus_message nextRemove();

                //Returns the first message in the queue
                cerberus_message next() const;

                //Returns the size of the queue
                size_t size() const;

                //Tells wether the queue is empty or not
                bool isEmpty() const;
        };
    }
}

#endif // CERBERUS_MESSAGE_MESSAGEQUEUE_H
