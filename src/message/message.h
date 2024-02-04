#ifndef CERBERUS_MESSAGE_MESSAGE_H
#define CERBERUS_MESSAGE_MESSAGE_H

#include <memory>
#include <vector>

#include "../Cerberus_global.h"
#include "../define.h"
#include "./slot/baseslot.h"

namespace cerberus
{
    namespace message
    {
        class CERBERUS_EXPORT Message
        {
           private:
            std::vector<cerberus_slot> m_slots;

            uint32_t m_id;

            uint32_t m_destinationId;

           public:
            static cerberus_message create(uint32_t id = CERBERUS_INVALID_ID);

            static cerberus_message createFrom(const Message& other);

            Message(uint32_t id = CERBERUS_INVALID_ID);

            size_t count() const;

            void addSlot(cerberus_slot slot);

            cerberus_slot getSlotAt(size_t index) const;

            cerberus_slot getSlot(const std::string& name) const;

            uint32_t id() const;

            uint32_t destination() const;

            void setDestination(uint32_t id);

            bool isValid() const;
        };
    }  // namespace message
}  // namespace cerberus

#endif  // CERBERUS_MESSAGE_MESSAGE_H
