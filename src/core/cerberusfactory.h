#ifndef CERBERUS_CORE_CERBERUSFACTORY_H
#define CERBERUS_CORE_CERBERUSFACTORY_H

#include "../message/message.h"

namespace cerberus
{
    namespace core
    {
        class CERBERUS_EXPORT CerberusFactory
        {
           public:
            CerberusFactory() = delete;

            CerberusFactory(const CerberusFactory& other) = delete;

            // Constructs a slot with the correct constructor according to the given type
            static message::slot::cerberus_slot slotConstruct(SlotType type);

            // Adds a template of the given message to the register, returning the chosen typeID
            static uint32_t registerMessage(const message::Message& message, const std::string& name = std::string());

            // Factory of messages. A call to this method will return an empty but structured message.
            // Will return an invalid message if typeID was not found, or if it's not a Message typeID.
            static message::cerberus_message messageConstruct(uint32_t id);

            // Factory of messages. A call to this method will return an empty but structured message.
            // Will return an invalid message if typeID was not found, or if it's not a Message typeID.
            static message::cerberus_message messageConstruct(const std::string& name);

            // Construct a standard message. See the StandardMessage enum
            static message::cerberus_message standardMessageConstruct(StandardMessage type);
        };
    }  // namespace core
}  // namespace cerberus

#endif  // CERBERUS_CORE_CERBERUSFACTORY_H
