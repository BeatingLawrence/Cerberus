#ifndef CERBERUS_MESSAGE_MESSAGETEMPLATE_H
#define CERBERUS_MESSAGE_MESSAGETEMPLATE_H

/*  This class represents a template of a message.
 *  It holds the message name and the list of its slots type and name.
 *  This class is used by the Cerberus factory to produce messages.
 */

#include <string>
#include <vector>

#include "../Cerberus_global.h"
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

        // Construct an empty template
        MessageTemplate();

        // Copy constructor
        MessageTemplate(const MessageTemplate& other) = default;

        // Construct an empty template with a name
        MessageTemplate(const std::string& name);

        // Construct a template taking data from a message
        MessageTemplate(const Message& message, const std::string& name);

        // Add a single slot type at the end of the template
        MessageTemplate& addSlotType(slot_ptr type, const std::string& name = "");

        // Return the slot at the index position
        slot_ptr getSlotAt(size_t index) const;

        // Return the number of slots contained in this template
        size_t count() const;

        // Return the name of the template
        std::string name() const;

        // Iterators
        ConstIterator<slot_ptr> begin() const;
        ConstIterator<slot_ptr> end() const;
    };
}  // namespace cerberus

#endif  // CERBERUS_MESSAGE_MESSAGETEMPLATE_H
