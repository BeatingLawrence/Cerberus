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
        struct SlotTemplate
        {
            SlotType type;
            std::string name;
        };

        std::vector<SlotTemplate> m_types;
        std::string m_name;

       public:
        HASH32 id;

        // Construct an empty template
        MessageTemplate();

        MessageTemplate(const MessageTemplate& other) = default;

        // Constructs an empty template with a name
        MessageTemplate(const std::string& name);

        // Constructs a template taking data from a message
        MessageTemplate(const Message& message, const std::string& name);

        // Adds a single slot type at the end of the template
        MessageTemplate& addSlotType(SlotType type, const std::string& name = "");

        // Returns the slot type at the index position
        SlotTemplate getSlotAt(size_t index) const;

        // Returns the number of slots contained in this template
        size_t count() const;

        std::string name() const;
    };
}  // namespace cerberus

#endif  // CERBERUS_MESSAGE_MESSAGETEMPLATE_H
