#ifndef CERBERUS_CORE_MESSAGEFACTORY_H
#define CERBERUS_CORE_MESSAGEFACTORY_H

#include "../register.h"
#include "../message/message.h"

namespace cerberus
{
    namespace core
    {
        class MessageFactory
        {
            private:
                Register m_register;

            public:
                MessageFactory();

                MessageFactory(const MessageFactory& other) = delete;

                //Constructs a slot with the correct constructor according to the given type
                message::slot::cerberus_slot slotFactory(message::slot::SlotType type);

                //Adds a template of the given message to the register, returning the chosen typeID
                uint32_t registerMessage(const message::Message& message, const std::string& name = std::string());

                //Returns a registered message ID searched by its name
                uint32_t messageIdByName(const std::string& name);

                //Factory of messages. A call to this method will return an empty but structured message.
                //Will throw an exception if typeID was not found, or if it's not a Message typeID.
                message::cerberus_message messageConstruct(uint32_t id);

                //Retrieves a Thread ID by its name
                uint32_t threadIdByName(const std::string& name);

                //Returns a generic registered object by its id. Returns nullptr if id was not found
                CerberusObject* cerberusObjectById(uint32_t id) const;

                //Checks if data are still present in the register. If they are, a delete will be called on them.
                //Note: Although they are still CerberusObjects, Threads are not considered in this call.
                void freeMemory();

                //Registers a given object and returns its id
                uint32_t registerCerberusObject(CerberusObject* object);

                //Unregister an object by its id
                void unregisterCerberusObject(uint32_t id);
        };
    }
}

#endif // CERBERUS_CORE_MESSAGEFACTORY_H
