#ifndef CERBERUS_CORE_CERBERUSFACTORY_H
#define CERBERUS_CORE_CERBERUSFACTORY_H

#include "../message/message.h"
#include "src/core/register.h"

namespace cerberus
{
    namespace core
    {
        class CerberusCore;

        class CERBERUS_EXPORT CerberusFactory
        {
            friend class ::cerberus::CerberusObject;
            friend class ::cerberus::core::CerberusCore;

           private:
            Register m_register;

            CerberusFactory();

            ~CerberusFactory();

            CerberusFactory(const CerberusFactory& other) = delete;

            // Registers a given object and returns its id
            static uint32_t _registerCerberusObject(CerberusObject* object);

            // Unregister an object by its id
            static void _unregisterCerberusObject(uint32_t id);

            // Returns a generic registered object by its id. Returns nullptr if id was not found
            static CerberusObject* _cerberusObjectById(uint32_t id);

            // Returns the singleton instance
            static CerberusFactory* _instance();

           public:
            enum StandardMessage
            {
                SM_LogMessage,
                SM_ShutdownMessage,
                // add more custom messages here
            };

            // Constructs a slot with the correct constructor according to the given type
            static message::slot::cerberus_slot slotFactory(SlotType type);

            // Adds a template of the given message to the register, returning the chosen typeID
            static uint32_t registerMessage(const message::Message& message, const std::string& name = std::string());

            // Returns a registered message ID searched by its name
            static uint32_t messageIdByName(const std::string& name);

            // Factory of messages. A call to this method will return an empty but structured message.
            // Will throw an invalid message if typeID was not found, or if it's not a Message typeID.
            static message::cerberus_message messageConstruct(uint32_t id);

            // Retrieves a Thread ID by its name
            static uint32_t threadIdByName(const std::string& name);

            static message::cerberus_message standardMessageConstruct(StandardMessage type);
        };
    }  // namespace core
}  // namespace cerberus

#endif  // CERBERUS_CORE_CERBERUSFACTORY_H
