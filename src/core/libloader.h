#ifndef CERBERUS_CORE_LIBLOADER_H
#define CERBERUS_CORE_LIBLOADER_H

#include "src/types.h"

namespace cerberus
{
    namespace core
    {
        class LibLoader
        {
           public:
            LibLoader();

            // Load a dynamic shared object.
            // If cerberus_own is true (default), then the object is owned by Cerberus and will be unloaded during
            // the framework de-init call. The user cannot unload it.
            // If cerberus_own is false, the object is not noted in the Cerberus registers so
            // the user must call unload() when the object it's not necessary anymore.
            // This call will return an ID of the object in the int parameter of the result.
            // This call can also be used to get the ID of an already loaded object.
            OperationResult load(const std::string& filename, bool cerberus_own = true);

            // Unload a dynamic shared object.
            OperationResult unload(int id);
        };
    }  // namespace core
}  // namespace cerberus

#endif  // CERBERUS_CORE_LIBLOADER_H
