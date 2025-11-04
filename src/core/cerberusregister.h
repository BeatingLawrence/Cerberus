#ifndef CERBERUS_CERBERUSREGISTER_H
#define CERBERUS_CERBERUSREGISTER_H

#include <list>
#include <string>

#include "../message/messagetemplate.h"
#include "../thread/mutex.h"
#include "../thread/mutexlocker.h"
#include "recordable.h"

namespace cerberus
{
    namespace core
    {
        class CerberusObject;

        class CerberusRegister
        {
            struct Plugin
            {
                Plugin(HASH32 id, void* h, const std::string& p)
                    : id(id),
                      handle(h),
                      path(p),
                      mutex() {};

                Plugin(Plugin&& other)
                    : id(other.id),
                      handle(other.handle),
                      path(other.path),
                      mutex(std::move(other.mutex))
                {
                    other.id     = 0;
                    other.handle = nullptr;
                    other.path   = "";
                };

                HASH32 id;
                void* handle;
                std::string path;
                Mutex mutex;
            };

            std::list<Recordable*> m_objects;
            Mutex m_objMutex;

            std::list<Plugin> m_plugins;
            Mutex m_pluginMutex;

            std::list<MessageTemplate> m_templates;
            Mutex m_tmpltMutex;

            HASH32 findAvailableObjId(const std::string& name);
            HASH32 findAvailableTmpltId(const std::string& name);
            HASH32 findAvailablePluginId(const std::string& path);

            // Give a cerberus object from its ID, or nullptr if it does not exist
            // This method does not lock the mutex!
            OpResData<Recordable*> objById(HASH32 id);

            // Give a cerberus object from its name, or nullptr if it does not exist
            // This method does not lock the mutex!
            OpResData<Recordable*> objByName(const std::string& name);

            // delete all the cerberus-owned objects
            void cleanup();

           public:
            CerberusRegister();

            ~CerberusRegister();

            static HASH32 removeReserved(HASH32 hash);

            //==================TEMPLATES====================

            // Retrieves a MessageTemplate by its name
            OpResData<MessageTemplate> msgTemplateByName(const std::string& name);

            // Retrieves a MessageTemplate by its ID
            OpResData<MessageTemplate> msgTemplateById(HASH32 id);

            // Register a new message template
            OpResData<HASH32> addMsgTemplate(const MessageTemplate& tmplt);

            //===================OBJECTS=====================

            // Register a given object and return its id
            // Return an invalid ID if the registering failed
            void registerObj(Recordable* object);

            // Unregiste an object by its id
            // Nothing happens if the ID does not exist
            void unregisterObj(HASH32 id);

            // Retrieves an object ID by its name
            HASH32 objIdByName(const std::string& name);

            // Send a message to a cerberus object.
            // If the id is not valid or the message cannot be sent, nothing happens
            void sendMsgToObj(HASH32 id, msg_ptr msg);
            void sendMsgToObj(const std::string& name, msg_ptr msg);

            //===================PLUGINS=====================

            // Add a plugin handle to the register. If the handle already exixst, exists is true
            // The new (or found) ID is returned
            HASH32 addPlugin(void* handle, const std::string& path, bool& exists);

            // Remove the handle from the register
            void removePlugin(HASH32 id);

            // Remove and unload all the loaded plugins
            void cleanupPlugins();

            // Return the requested handle if it is registered, otherwise nullptr
            void* checkPlugin(HASH32 id);

            // Get the mutexlocker of a loaded shared object. The mutex is locked before return
            MutexLocker getPluginMutex(HASH32 id);

            // Replaces data of an existing plugin. Returns false if id does not exist, true otherwise
            bool updatePlugin(HASH32 id, const std::string& path, void* handle);
        };
    }  // namespace core
}  // namespace cerberus

#endif  // CERBERUS_CERBERUSREGISTER_H
