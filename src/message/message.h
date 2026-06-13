#ifndef CERBERUS_MESSAGE_MESSAGE_H
#define CERBERUS_MESSAGE_MESSAGE_H

#include <vector>

#include "../Cerberus_global.h"
#include "../define.h"
#include "../types.h"
#include "slot.h"  // IWYU pragma: export

namespace crb
{
    class Message : public Clonable
    {
       private:
        mutable std::vector<slot_ptr> m_slots;

        HASH32 m_id;

        mutable std::vector<HASH32> m_recipientIds;

        CERBERUS_EXPORT slot_ptr* _slot(const std::string& name);
        CERBERUS_EXPORT const slot_ptr* _slot(const std::string& name) const;

        Message(HASH32 id = CRB_INVALID_ID);

        Message(const Message& other);

        Message& operator=(const Message& other) = delete;

       public:
        static CERBERUS_EXPORT msg_ptr create(HASH32 id = CRB_INVALID_ID);

        CERBERUS_EXPORT virtual ~Message();

        // get the number of slots
        CERBERUS_EXPORT size_t count() const;

        // add a slot copying it
        CERBERUS_EXPORT Message& addSlot(const slot_ptr& slot);

        // add a slot moving it
        CERBERUS_EXPORT Message& addSlot(slot_ptr&& slot);

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
        CERBERUS_EXPORT slot_ptr& getSlotAt(size_t index);
        CERBERUS_EXPORT slot_ptr getSlotAt(size_t index) const;
        CERBERUS_EXPORT slot_ptr& getSlot(const std::string& name);
        CERBERUS_EXPORT slot_ptr getSlot(const std::string& name) const;

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

            const slot_ptr* p = _slot(name);
            if (!p) throw cIllegalArgExc("Slot \"%s\" not found", name.c_str());
            const auto* slot = (*p)->to<T>();
            return (slot->value());
        }

        // check if a slot exists and can be cast to the requested slot type
        template <typename T>
        bool has(const std::string& name) const
        {
            static_assert(std::is_base_of<SlotBase, T>::value,
                          "Message::has<TSlot>: TSlot must derive from SlotBase");

            const slot_ptr* slot = _slot(name);
            if (!slot || !(*slot)) return false;
            try
            {
                (*slot)->to<T>();
            }
            catch (...)
            {
                return false;
            }
            return true;
        }

        // return the ID of this message
        CERBERUS_EXPORT HASH32 id() const;

        // checks if the ID of this equals the given ID
        CERBERUS_EXPORT bool is(HASH32 id) const;

        // return the first recipient of the message (if any)
        CERBERUS_EXPORT HASH32 recipient() const;

        // get all recipients
        CERBERUS_EXPORT const std::vector<HASH32>& recipients() const;

        // check if at least one recipient is valid
        CERBERUS_EXPORT bool hasValidRecipient() const;

        // set a single recipient (clears any existing recipients)
        CERBERUS_EXPORT void setRecipient(HASH32 id) const;

        // set recipients
        CERBERUS_EXPORT void setRecipients(const std::vector<HASH32>& ids) const;

        // add a recipient
        CERBERUS_EXPORT void addRecipient(HASH32 id) const;

        // clear all recipients
        CERBERUS_EXPORT void clearRecipients() const;

        // convert the message to a plain buffer
        CERBERUS_EXPORT ByteBuffer toBuffer() const;

        // iterators
        CERBERUS_EXPORT ConstIterator<slot_ptr> begin() const;
        CERBERUS_EXPORT ConstIterator<slot_ptr> end() const;

        // clone this message and all of its slots
        CERBERUS_EXPORT virtual Clonable* clone() const;

        // calculate the memory footprint of this message, iterating over all of its slots
        CERBERUS_EXPORT virtual LSIZE memfp() const;
    };
}  // namespace crb

#endif  // CERBERUS_MESSAGE_MESSAGE_H
