#ifndef CERBERUS_MESSAGE_MESSAGE_H
#define CERBERUS_MESSAGE_MESSAGE_H

#include <vector>

#include "../Cerberus_global.h"
#include "../define.h"
#include "../types.h"
#include "slot.h"  // IWYU pragma: export

namespace cerberus
{
    class CERBERUS_EXPORT Message : public Clonable
    {
       private:
        mutable std::vector<slot_ptr> m_slots;

        HASH32 m_id;

        mutable std::vector<HASH32> m_recipientIds;

        slot_ptr* _slot(const std::string& name) const;

        Message(HASH32 id = CERBERUS_INVALID_ID);

        Message(const std::string& name);

        Message(const Message& other);

        Message& operator=(const Message& other) = delete;

       public:
        static msg_ptr create(HASH32 id = CERBERUS_INVALID_ID);
        static msg_ptr create(const std::string& name);

        virtual ~Message();

        // get the number of slots
        size_t count() const;

        // add a slot copying it
        Message& addSlot(const slot_ptr& slot);

        // add a slot moving it
        Message& addSlot(slot_ptr&& slot);

        // add a slot specifying just the type and name (no content)
        template <typename T>
        Message& addSlotType(const std::string& name = "")
        {
            static_assert(std::is_base_of<SlotBase, T>::value,
                          "MessageTemplate::addSlotType<T>: T must derive from SlotBase");

            slot_ptr p = T::create();
            p->setId(name);
            m_slots.push_back(std::move(p));
            return *this;
        }

        // get a slot
        slot_ptr& getSlotAt(size_t index);
        slot_ptr getSlotAt(size_t index) const;
        slot_ptr& getSlot(const std::string& name);
        slot_ptr getSlot(const std::string& name) const;

        // quick access helper. Access directly to the underlying object of one slot (value type)
        template <typename T>
        decltype(auto) get(const std::string& name)
        {
            static_assert(std::is_base_of<SlotBase, T>::value,
                          "Message::get<TSlot>: TSlot must derive from SlotBase");

            auto* slot = getSlot(name)->to<T>();
            return (slot->value());
        }

        template <typename T>
        decltype(auto) get(const std::string& name) const
        {
            static_assert(std::is_base_of<SlotBase, T>::value,
                          "Message::get<TSlot>: TSlot must derive from SlotBase");

            const auto* slot = getSlot(name)->to<T>();
            return (slot->value());
        }

        // return the ID of this message
        HASH32 id() const;

        // convert a textual name to an ID
        static HASH32 idFromName(const std::string& name);

        // checks if the ID of this equals the idFromName(name)
        bool is(const std::string& name) const;

        // return the first recipient of the message (if any)
        HASH32 recipient() const;

        // get all recipients
        const std::vector<HASH32>& recipients() const;

        // check if at least one recipient is valid
        bool hasValidRecipient() const;

        // set a single recipient (clears any existing recipients)
        void setRecipient(HASH32 id) const;

        // set recipients
        void setRecipients(const std::vector<HASH32>& ids) const;

        // add a recipient
        void addRecipient(HASH32 id) const;
        OpRes addRecipient(const std::string& name) const;

        // clear all recipients
        void clearRecipients() const;

        // convert the message to a plain buffer
        ByteBuffer toBuffer() const;

        // iterators
        ConstIterator<slot_ptr> begin() const;
        ConstIterator<slot_ptr> end() const;

        // clone this message and all of its slots
        virtual Clonable* clone() const;

        // calculate the memory footprint of this message, iterating over all of its slots
        virtual SIZE memfp() const;
    };
}  // namespace cerberus

#endif  // CERBERUS_MESSAGE_MESSAGE_H
