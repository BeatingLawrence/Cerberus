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
    class CerberusObject;

    namespace core
    {
        class LibLoader;
        class CerberusCore;
        class CerberusRegister
        {
            friend class ::cerberus::CerberusObject;
            friend class ::cerberus::core::LibLoader;
            friend class ::cerberus::core::CerberusCore;

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

            mutex::Mutex mutex;

            CerberusRegister();

            CerberusRegister(const CerberusRegister& other) = delete;

            // Get the singleton instance
            static CerberusRegister& instance();

            // Register a given object and return its id
            // Return an invalid ID if the registering failed
            static void registerObj(CerberusObject* object);

            // Unregiste an object by its id
            // Nothing happens if the ID does not exist
            static void unregisterObj(uint32_t id);

            // Add a plugin handle to the register. If the handle already exixst, exists is true
            // The new (or found) ID is returned
            static uint32_t addPlugin(void* handle, const std::string& path, bool& exists);

            // Remove the handle from the register
            static void removePlugin(uint32_t id);

            // Remove and unload all the loaded plugins
            static void cleanupPlugins();

            // Return the requested handle if it is registered, otherwise nullptr
            static void* checkPlugin(uint32_t id);

            // Get the mutexlocker of a loaded shared object. The mutex is locked before return
            static mutex::MutexLocker getPluginMutex(uint32_t id);

            // Replaces data of an existing plugin. Returns false if id does not exist, true otherwise
            static bool updatePlugin(uint32_t id, const std::string& path, void* handle);

            // Give a cerberus object from its ID, or nullptr if it does not exist
            // This method does not lock the mutex!
            CerberusObject* objById(uint32_t id);

            // Give a cerberus object from its name, or nullptr if it does not exist
            // This method does not lock the mutex!
            CerberusObject* objByName(const std::string& name);

           public:
            ~CerberusRegister();

            // Retrieves a Thread ID by its name
            static uint32_t threadIdByName(const std::string& name);

            // Retrieves a MessageTemplate by its name
            static message::MessageTemplate msgTemplateByName(const std::string& name);

            // Retrieves a MessageTemplate by its ID
            static message::MessageTemplate msgTemplateById(uint32_t id);

            // Send a message to a cerberus object.
            // If the id is not valid or the message cannot be sent, nothing happens
            static void sendMsgToObj(uint32_t id, message::cerberus_message msg);
        };
    }  // namespace core
}  // namespace cerberus

#endif  // CERBERUS_CERBERUSREGISTER_H
