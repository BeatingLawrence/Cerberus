#ifndef CERBERUS_MESSAGE_MESSAGETEMPLATE_H
#define CERBERUS_MESSAGE_MESSAGETEMPLATE_H

/*  This class represents a template of a message.
 *  It holds the message name and the set of its slot types.
 *  This class is used by the Messages factory to produce standardized and registered messages.
 */

#include <string>
#include <vector>

#include "../Cerberus_global.h"
#include "../core/cerberusobject.h"

namespace cerberus
{
    namespace message
    {
        class Message;

        class CERBERUS_EXPORT MessageTemplate : public core::CerberusObject
        {
           private:
            std::vector<SlotType> m_types;

           public:
            // Construct an invalid template
            MessageTemplate();

            // Constructs an empty template with a name
            MessageTemplate(const std::string& name);

            // Constructs a template taking data from a message
            MessageTemplate(const Message& message, const std::string& name);

            virtual ~MessageTemplate();

            // Adds a single slot type at the end of the vector
            MessageTemplate& addSlotType(SlotType type);

            // Returns the slot type at the index position
            SlotType getSlotTypeAt(size_t index) const;

            // Returns the number of slots contained in this template
            size_t count() const;
        };
    }  // namespace message
}  // namespace cerberus

#endif  // CERBERUS_MESSAGE_MESSAGETEMPLATE_H
