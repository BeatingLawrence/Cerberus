#ifndef CERBERUS_REGISTER_H
#define CERBERUS_REGISTER_H

/*  This class holds all Cerberus registers, and generates the IDs of the new entries.
 *
 *  Registers are:
 *      1. Message templates register
 *      2. Threads register
 *      3. Sockets register
 *      ...
 *
 *  This class is not supposed to be used directly. An instance of this class
 *  will be created and managed by the Cerberus provider
 */

#include "./Cerberus_global.h"
#include "./message/messagetemplate.h"
#include "./mutex/mutex.h"
#include <list>

namespace cerberus
{
    class CERBERUS_EXPORT Register
    {
        private:
            std::list<std::pair<uint32_t, message::MessageTemplate>> m_messageTemplates;

            uint32_t _findAvailableId_messageTemplates();

            mutable mutex::Mutex m_messageTemplateMutex;

            mutable mutex::Mutex m_threadMutex;

            mutable mutex::Mutex m_socketMutex;

        public:
            static constexpr uint32_t Invalid_ID = 0;

            Register();

            Register(const Register& other) = delete;

            //Message template section (m_messageTemplateMutex):    ===================================================

            //Adds a new message template to the structure and returns the chosen ID.
            uint32_t addMessageTemplate(const message::MessageTemplate& toAdd);

            //Removes message template with given id. Will throw an exception if id does not exist
            void removeMessageTemplate(uint32_t idToRemove);

            //Finds a message ID by its name. Returns an invalid ID if search fails
            uint32_t messageIdByName(const std::string& name) const;

            //Returns message template with given id. Will throw an exception if id does not exist
            message::MessageTemplate messageTemplateById(uint32_t id) const;

            //Checks if a template with given name already exists in the structure. Returns true if it does
            bool messageTemplateNameAlreadyExists(const std::string& name) const;

            //Threads section (m_threadMutex):                      ===================================================

            //...

            //Sockets section (m_socketMutex):                      ===================================================

            //...
    };

} // namespace cerberus

#endif // CERBERUS_REGISTER_H
