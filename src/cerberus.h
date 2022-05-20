#ifndef CERBERUS_H
#define CERBERUS_H

#include <string>
#include "Cerberus_global.h"
#include "./core/cerberuscore.h"
#include "./core/cerberuslog.h"
#include "./core/messagefactory.h"

namespace cerberus
{
    class CERBERUS_EXPORT Cerberus : public core::CerberusCore
    {
            friend class ::cerberus::CerberusObject;
            friend class ::cerberus::core::CerberusLog;

        private:
            Cerberus();

            Cerberus(const Cerberus& other) = delete;
            Cerberus(const Cerberus&& other) = delete;
            void operator=(const Cerberus& other) = delete;

            static Cerberus* _instance();

            bool m_initFlag;

            core::MessageFactory m_factory;

            core::CerberusLog m_log;

            mutex::Mutex m_mutex;

            ~Cerberus();

            CerberusObject* cerberusObjectById(uint32_t id) override;

            void freeRegisterMemory() override;

            static uint32_t _registerCerberusObject(CerberusObject* object);

            static void _unregisterCerberusObject(uint32_t id);

            static core::CerberusLog* _logInstance();

        public:
            //Performs the init sequence of the Cerberus framework. This operation must precede any others [TRANSFERS]
            static void init(const CerberusInitParms* parms);

            //Performs the de-init sequence of the Cerberus framework. (TBC)
            static void deinit();

            //Returns a working set of default parameters that can be a start point for customization [TRANSFERS]
            static CerberusInitParms* cerberusDefaultParms();

            //Adds a template of the given message to the register, returning the chosen typeID
            static uint32_t registerMessage(const message::Message& message, const std::string& name = std::string());

            //Returns a registered message ID searched by its name
            static uint32_t messageIdByName(const std::string& name);

            //Factory of messages. A call to this method will return an empty but structured message.
            //Will throw an exception if typeID was not found, or if it's not a Message typeID.
            static message::cerberus_message messageConstruct(uint32_t id);

            //Sends a message
            static void send(message::cerberus_message message);

            //Retrieves a Thread ID by its name
            static uint32_t threadIdByName(const std::string& name);
    };
}

#endif // CERBERUS_H
