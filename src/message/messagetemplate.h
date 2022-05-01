#ifndef CERBERUS_MESSAGE_MESSAGETEMPLATE_H
#define CERBERUS_MESSAGE_MESSAGETEMPLATE_H

/*  This class represents a template of a message.
 *  It holds the message name, id and the set of its slot types.
 *  This class is used by the Messages factory to produce standardized and registered messages.
 */

#include "./slot/baseslot.h"
#include "./message.h"
#include <vector>
#include <string>
#include "../Cerberus_global.h"

namespace cerberus
{
    namespace message
    {
        class CERBERUS_EXPORT MessageTemplate
        {
            private:
                std::vector<slot::BaseSlot::SlotType> m_types;

                std::string m_name;

            public:
                MessageTemplate() = delete;

                MessageTemplate(const MessageTemplate& other) = default;

                //Constructs an empty template with a name and an ID
                MessageTemplate(const std::string& name);

                //Constructs a template taking data from a message
                MessageTemplate(const Message& message, const std::string& name);

                //Adds a single slot type at the end of the vector
                void addSlotType(slot::BaseSlot::SlotType type);

                //Returns the slot type at the index position
                slot::BaseSlot::SlotType getSlotTypeAt(size_t index) const;

                //Returns the template name
                std::string name() const;

                //Returns the number of slots contained in this template
                uint32_t count() const;
        };
    }
}

#endif // CERBERUS_MESSAGE_MESSAGETEMPLATE_H
