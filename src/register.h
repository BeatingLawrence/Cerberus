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
    namespace thread
    {
        class Thread;
    }

    class CERBERUS_EXPORT Register
    {
        private:
            struct MessageTemplateEntry
            {
                uint32_t typeID;
                message::MessageTemplate tmplate;
            };

            struct ThreadEntry
            {
                uint32_t threadID;
                std::string name;
                cerberus::thread::Thread* thread;
            };

            std::list<MessageTemplateEntry> m_messageTemplates;

            std::list<ThreadEntry> m_threads;

            uint32_t _findAvailableTypeID_messageTemplates();

            uint32_t _findAvailableID_threads();

            mutable mutex::Mutex m_messageTemplateMutex;

            mutable mutex::Mutex m_threadMutex;

            mutable mutex::Mutex m_socketMutex;

        public:
            Register();

            Register(const Register& other) = delete;

            //Message template section (m_messageTemplateMutex):    ===================================================

            //Adds a new message template to the structure and returns the chosen typeID.
            //Returns an invalid typeID if name is already registered.
            uint32_t addMessageTemplate(const message::MessageTemplate& toAdd);

            //Removes message template with given typeID. Will throw an exception if typeID does not exist
            void removeMessageTemplate(uint32_t typeID);

            //Finds a message typeID by its name. Returns an invalid typeID if search fails
            uint32_t messageTypeIdByName(const std::string& name) const;

            //Returns message template with given id. Will throw an exception if id does not exist
            message::MessageTemplate messageTemplateByTypeId(uint32_t typeID) const;

            //Checks if a template with given name already exists in the structure. Returns true if it does
            bool messageTemplateNameAlreadyExists(const std::string& name) const;

            //Threads section (m_threadMutex):                      ===================================================

            //Adds a new thread to the structure and returns the chosen ID.
            //Returns an invalid ID if name is already registered.
            uint32_t addThread(cerberus::thread::Thread* thread, const std::string& name);

            //Removes thread with given ID. Will throw an exception if ID does not exist
            void removeThread(uint32_t id);

            //Finds thread ID by its name. Returns an invalid ID if search fails
            uint32_t threadIdByName(const std::string& name) const;

            //Returns a pointer of the Thread with given id. Will throw an exception if id does not exist
            cerberus::thread::Thread* threadById(uint32_t id) const;

            //Checks if a Thread with given name already exists in the structure. Returns true if it does
            bool threadNameAlreadyExists(const std::string& name) const;

            //Sockets section (m_socketMutex):                      ===================================================

            //...
    };

} // namespace cerberus

#endif // CERBERUS_REGISTER_H
