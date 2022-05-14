#ifndef CERBERUS_MESSAGE_MESSAGE_H
#define CERBERUS_MESSAGE_MESSAGE_H

#include <memory>
#include <vector>
#include "./slot/baseslot.h"
#include "../Cerberus_global.h"

namespace cerberus
{
    namespace message
    {
        typedef std::shared_ptr<class Message> cerberus_message;
        typedef std::shared_ptr<const class Message> cerberus_const_message;

        class CERBERUS_EXPORT Message
        {
            private:
                std::vector<slot::cerberus_slot> m_slots;

                uint32_t m_id;

            public:
                static cerberus_message create();

                static cerberus_message createFrom(const Message& other);

                Message();

                Message(const Message& other);

                size_t count() const;

                void addSlot(slot::cerberus_slot slot);

                slot::cerberus_slot getSlotAt(size_t index) const;

                slot::cerberus_slot getSlotById(uint32_t id) const;

                void setId(uint32_t id);

                uint32_t id() const;

                bool isValid() const;
        };
    }
}

#endif // CERBERUS_MESSAGE_MESSAGE_H
