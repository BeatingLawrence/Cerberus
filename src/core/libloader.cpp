#include "libloader.h"

#include <dlfcn.h>

using namespace cerberus::core;

//=============================================================================
LibLoader::LibLoader() {}
//=============================================================================
cerberus::OperationResult LibLoader::load(const std::string &filename, bool cerberus_own)
{
    // check if the object is already loaded
    auto p = dlopen(filename.c_str(), RTLD_NOLOAD | RTLD_NOW);

    if (p == nullptr)  // the object was not loaded yet
    {
    }
}
//=============================================================================
cerberus::OperationResult LibLoader::unload(int id) {}
//=============================================================================

// WIP
