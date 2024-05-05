#ifndef CERBERUS_CORE_LIBLOADER_H
#define CERBERUS_CORE_LIBLOADER_H

#include "../types.h"

/*  This is the LibLoader class
 *
 *  This class is used for loading a shared library (aka plugin)
 *  inside the current process image space.
 *
 *  The LibLoader works in one of these modes:
 *
 *  standalone - After creation, the loader calls load() with the noreg = true.
 *               In this way, the library is loaded in the process image space
 *               and is owned by the owner of the LibLoader instance.
 *               As soon as the owner calls unload() (or destroys the instance) the
 *               plugin is unloaded. When the library is loaded, an unload() attempt from
 *               another thread cannot happen in this mode.
 *
 *  shared     - After creation, the loader calls load() with the noreg = false (default).
 *               In this way, the library is loaded in the process image space and is noted
 *               in the Cerberus register. After this operation, the LibLoader instance has just
 *               a reference to the shared library, but it has no ownership, so it cannot unload it anymore.
 *               The Cerberus framework will automatically unload all the shared objects when deinit()
 *               will be called.
 *               When another thread calls load() on the same plugin file, the plugin will not be loaded,
 *               instead, the same reference will be gotten from the Cerberus framework register.
 *
 *               The swap() method:
 *
 *               The shared mode has the advantage of having the swap() method. This method unloads the
 *               current plugin, and reloads another plugin in place of the unloaded one. This operation
 *               is thread safe and is useful for changing the plugin implementation without restarting
 *               the application and without implementing a synchronization mechanism.
 *               The reload() method does the same thing as swap(), but it just uses the same path
 *
 */

namespace cerberus
{
    namespace core
    {
        class LibLoader
        {
           private:
            uint32_t m_id;
            void* m_handle;
            bool m_noreg;
            std::string m_path;

            static OpRes close(void* handle);
            OpRes open(const std::string& path);
            bool isLoaded(const std::string& path) const;

           public:
            LibLoader();

            ~LibLoader();

            // Load a dynamic shared object. This call will link this instance of LibLoader to a
            // defined library image (if the load process completes successfully).
            // If noreg is false (default), then the object is owned by Cerberus and will be unloaded during
            // the framework de-init call. The application cannot unload it.
            // If noreg is true, the object is not noted in the Cerberus registers so
            // the user must call unload() when the object it's not necessary anymore.
            // This call can also be used to setup a LibLoader instance for an already loaded object
            OpRes load(const std::string& path, bool noreg = false);

            // Unload a dynamic shared object.
            // This method only works in standalone mode
            OpRes unload();

            // Swap the loaded library with the image provided at the given path.
            // The operation is thread-safe and may be done while other threads are using the image
            // This method only works in shared mode
            OpRes swap(const std::string& path);

            // Reload the plugin. This method is equal to swap(), just without changing path
            OpRes reload();

            // Get the pointer to an object contained in the plugin.
            // If an error occurs, an invalid LoaderFunc is returned.
            // The shared object mutex remains locked until the locker instance inside the
            // returned LoaderFunc is destroyed
            LoaderFunc get(const std::string& symbol);

            // Check if the image has been loaded in the process image
            bool isLoaded() const;

            // Load a shared object into the Cerberus framework domain, avoiding
            // to manually create a LibLoader instance
            static OpRes fastload(const std::string& path);
        };
    }  // namespace core
}  // namespace cerberus

#endif  // CERBERUS_CORE_LIBLOADER_H
