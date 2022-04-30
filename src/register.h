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
#include <list>

namespace cerberus
{
    class CERBERUS_EXPORT Register
    {
        private:
            std::list<message::MessageTemplate> m_messageTemplates;

            uint32_t _findAvailableId_messageTemplates();

        public:
            Register();

            Register(const Register& other) = delete;

            //Adds a new message template to the structure and returns the chosen ID.
            uint32_t addMessageTemplate(const message::MessageTemplate& toAdd);

            //Removes message template with given id. Will throw an exception if id does not exist
            void removeMessageTemplate(uint32_t idToRemove);

            //Returns message template with given id. Will throw an exception if id does not exist
            message::MessageTemplate messageTemplateById(uint32_t id) const;
    };

} // namespace cerberus

#endif // CERBERUS_REGISTER_H
