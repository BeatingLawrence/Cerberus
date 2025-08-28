#ifndef CERBERUS_MESSAGE_MESSAGETEMPLATE_H
#define CERBERUS_MESSAGE_MESSAGETEMPLATE_H

/*  This class represents a template of a message.
 *  It holds the message name and the list of its slots type and name.
 *  This class is used by the Cerberus factory to produce messages.
 */

#include <string>
#include <vector>

#include "../Cerberus_global.h"
#include "../message/slot.h"
#include "../types.h"

namespace cerberus
{
    class Message;

    class CERBERUS_EXPORT MessageTemplate
    {
        std::vector<slot_ptr> m_types;
        std::string m_name;

       public:
        HASH32 id;

        // Construct an empty template with given ID
        MessageTemplate(HASH32 id = CERBERUS_INVALID_ID);

        // Copy constructor
        MessageTemplate(const MessageTemplate& other);

        // Construct an empty template with a name
        MessageTemplate(const std::string& name);

        // Construct a template taking data from a message
        MessageTemplate(const Message& message, const std::string& name);

        // Add a single slot type at the end of the template
        template <typename T>
        MessageTemplate& addSlotType(const std::string& name = "")
        {
            static_assert(std::is_base_of<SlotBase, T>::value,
                          "MessageTemplate::addSlotType<T>: T must derive from SlotBase");

            slot_ptr proto(new T());

            m_types.push_back(proto->newslot());
            m_types.back()->setId(name);
            return *this;
        }

        // Return the slot at the index position
        slot_ptr& getSlotAt(size_t index);

        // Return the number of slots contained in this template
        size_t count() const;

        // Return the name of the template
        std::string name() const;

        // Iterators
        ConstIterator<slot_ptr> begin() const;
        ConstIterator<slot_ptr> end() const;

        msg_ptr instantiate() const;
    };
}  // namespace cerberus

#endif  // CERBERUS_MESSAGE_MESSAGETEMPLATE_H
