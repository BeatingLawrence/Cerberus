#ifndef CERBERUS_CERBERUSREGISTER_H
#define CERBERUS_CERBERUSREGISTER_H

#include <cstdint>
#include <list>
#include <string>

#include "../message/message.h"
#include "../message/messagetemplate.h"
#include "../mutex/mutex.h"
#include "../mutex/mutexlocker.h"

namespace cerberus
{
    namespace core
    {
        class CerberusObject;

        class CerberusRegister
        {
            struct Plugin
            {
                Plugin(uint32_t id, void* h, const std::string& p)
                    : id(id),
                      handle(h),
                      path(p),
                      mutex(){};

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

                uint32_t id;
                void* handle;
                std::string path;
                mutex::Mutex mutex;
            };

           private:
            std::list<CerberusObject*> m_objects;

            std::list<Plugin> m_plugins;

            uint32_t findAvailableId();

            uint32_t findAvailablePluginId();

            mutex::Mutex m_mutex;

            // Give a cerberus object from its ID, or nullptr if it does not exist
            // This method does not lock the mutex!
            CerberusObject* objById(uint32_t id);

            // Give a cerberus object from its name, or nullptr if it does not exist
            // This method does not lock the mutex!
            CerberusObject* objByName(const std::string& name);

           public:
            CerberusRegister();

            // Register a given object and return its id
            // Return an invalid ID if the registering failed
            void registerObj(CerberusObject* object);

            // Unregiste an object by its id
            // Nothing happens if the ID does not exist
            void unregisterObj(uint32_t id);

            // Add a plugin handle to the register. If the handle already exixst, exists is true
            // The new (or found) ID is returned
            uint32_t addPlugin(void* handle, const std::string& path, bool& exists);

            // Remove the handle from the register
            void removePlugin(uint32_t id);

            // Remove and unload all the loaded plugins
            void cleanupPlugins();

            // Return the requested handle if it is registered, otherwise nullptr
            void* checkPlugin(uint32_t id);

            // Get the mutexlocker of a loaded shared object. The mutex is locked before return
            mutex::MutexLocker getPluginMutex(uint32_t id);

            // Replaces data of an existing plugin. Returns false if id does not exist, true otherwise
            bool updatePlugin(uint32_t id, const std::string& path, void* handle);

            // Retrieves an object ID by its name
            uint32_t objIdByName(const std::string& name);

            // Retrieves a MessageTemplate by its name
            message::MessageTemplate msgTemplateByName(const std::string& name);  // consider hiding

            // Retrieves a MessageTemplate by its ID
            message::MessageTemplate msgTemplateById(uint32_t id);  // consider hiding

            // Send a message to a cerberus object.
            // If the id is not valid or the message cannot be sent, nothing happens
            void sendMsgToObj(uint32_t id, cerberus_message msg);
        };
    }  // namespace core
}  // namespace cerberus

#endif  // CERBERUS_CERBERUSREGISTER_H
