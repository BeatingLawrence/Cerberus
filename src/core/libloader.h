#ifndef CERBERUS_CORE_LIBLOADER_H
#define CERBERUS_CORE_LIBLOADER_H

#include "src/types.h"

namespace cerberus
{
    namespace core
    {
        class LibLoader
        {
           private:
            void* m_handle;
            bool m_noreg;
            std::string m_path;

            static OperationResult close(void* handle);
            OperationResult open(const std::string& path);
            bool isLoaded(const std::string& path) const;

           public:
            LibLoader();

            // Load a dynamic shared object. This call will link this instance of LibLoader to a
            // defined library image (if the load process completes successfully).
            // If noreg is false (default), then the object is owned by Cerberus and will be unloaded during
            // the framework de-init call. The application cannot unload it.
            // If noreg is true, the object is not noted in the Cerberus registers so
            // the user must call unload() when the object it's not necessary anymore.
            // This call can also be used to setup a LibLoader instance for an already loaded object
            OperationResult load(const std::string& path, bool noreg = false);

            // Unload a dynamic shared object. This method only works if the shared object
            // has been loaded with noreg = true
            OperationResult unload();

            // Get the pointer to an object contained in the plugin.
            // If an error occurs, an invalid LoaderFunc is returned.
            // The shared object mutex remains locked until the locker instance inside the
            // returned LoaderFunc is destroyed
            LoaderFunc get(const std::string& symbol);

            // Check if the image has been loaded in the process image
            bool isLoaded() const;

            // Load a shared object into the Cerberus framework domain, avoiding
            // the creation of a LibLoader instance
            static OperationResult fastload(const std::string& path);
        };
    }  // namespace core
}  // namespace cerberus

#endif  // CERBERUS_CORE_LIBLOADER_H
