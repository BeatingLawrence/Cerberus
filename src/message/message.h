#ifndef CERBERUS_MESSAGE_MESSAGE_H
#define CERBERUS_MESSAGE_MESSAGE_H

#include <vector>

#include "../Cerberus_global.h"
#include "../define.h"
#include "../types.h"
#include "slot/slot.h"  // IWYU pragma: export

namespace cerberus
{
    class CERBERUS_EXPORT Message : public Clonable
    {
       private:
        std::vector<slot_ptr> m_slots;

        HASH32 m_id;

        HASH32 m_recipientId;

       public:
        static cerberus_message create(HASH32 id = CERBERUS_INVALID_ID);

        Message(HASH32 id = CERBERUS_INVALID_ID);

        Message(const Message& other) = default;

        Message& operator=(const Message& other) = delete;

        virtual ~Message();

        size_t count() const;

        Message& addSlot(slot_ptr slot);

        Message& clear();

        slot_ptr getSlotAt(size_t index);

        slot_ptr getConstSlotAt(size_t index) const;

        slot_ptr getSlot(const std::string& name);

        slot_ptr getConstSlot(const std::string& name) const;

        HASH32 id() const;

        HASH32 recipient() const;

        bool hasValidRecipient() const;

        Message& setRecipient(HASH32 id);

        bool isValid() const;

        virtual Clonable* clone() const;

        // Fill the fields.
        // Given value types must match with the actual value types
        // stored in the message, otherwise a cast exception
        // for the corresponding slot will be thrown
        Message& fill(std::initializer_list<TypeWrapper> values);

        // Insert the specified values in the fields.
        // This method is similar to fill() except for the fact that insert()
        // clears the message first, deleting all slots and then re-creates them
        // one by one using the given type and value.
        Message& insert(std::initializer_list<TypeWrapper> values);
    };
}  // namespace cerberus

#endif  // CERBERUS_MESSAGE_MESSAGE_H
