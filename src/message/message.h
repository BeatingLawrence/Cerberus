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

        HASH32 m_recipientId;

        slot_ptr* _slot(const std::string& name) const;

       public:
        static msg_ptr create(HASH32 id = CERBERUS_INVALID_ID);
        static msg_ptr create(const std::string& name);

        Message(HASH32 id = CERBERUS_INVALID_ID);

        Message(const std::string& name);

        Message(const Message& other);

        Message& operator=(const Message& other) = delete;

        virtual ~Message();

        size_t count() const;

        // add a slot moving it
        Message& addSlot(slot_ptr&& slot);

        // add a slot specifying just the type and name (no content)
        template <typename T>
        Message& addSlotType(const std::string& name = "")
        {
            static_assert(std::is_base_of<SlotBase, T>::value,
                          "MessageTemplate::addSlotType<T>: T must derive from SlotBase");

            slot_ptr p(new T());
            p->setId(name);
            m_slots.push_back(std::move(p));
            return *this;
        }

        slot_ptr& getSlotAt(size_t index);

        slot_ptr getSlotAt(size_t index) const;

        slot_ptr& getSlot(const std::string& name);

        slot_ptr getSlot(const std::string& name) const;

        template <typename T>
        slot_ptr& get(const std::string& name)
        {
            return getSlot(name)->to<T>();
        }

        HASH32 id() const;

        bool is(const std::string& name) const;

        HASH32 recipient() const;

        bool hasValidRecipient() const;

        Message& setRecipient(HASH32 id);

        ByteBuffer toBuffer() const;

        ConstIterator<slot_ptr> begin() const;
        ConstIterator<slot_ptr> end() const;

        virtual Clonable* clone() const;

        virtual SIZE memfp() const;
    };
}  // namespace cerberus

#endif  // CERBERUS_MESSAGE_MESSAGE_H
