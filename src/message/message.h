#ifndef CERBERUS_MESSAGE_MESSAGE_H
#define CERBERUS_MESSAGE_MESSAGE_H

#include <memory>
#include <vector>
#include "./slot/baseslot.h"

namespace cerberus
{
    namespace message
    {
        typedef std::shared_ptr<class Message> cerberus_message;
        typedef std::shared_ptr<const class Message> cerberus_const_message;

        class Message
        {
            private:
                std::vector<slot::cerberus_slot> m_slots;

            public:
                static cerberus_message create();

                static cerberus_message create(const Message& other);

                Message();

                Message(const Message& other);

                size_t count() const;

                void addSlot(slot::cerberus_slot slot);

                slot::cerberus_slot getSlotAt();
        };
    }
}

#endif // CERBERUS_MESSAGE_MESSAGE_H
