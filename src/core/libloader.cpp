#include "libloader.h"

#include <dlfcn.h>

#include "src/core/cerberuslog.h"
#include "src/core/cerberusregister.h"

using namespace cerberus::core;

//=============================================================================
cerberus::OperationResult LibLoader::close(void* handle)
{
    if (dlclose(handle) != 0)
    {
        // error
        clogError("Error while calling dlclose: %s", dlerror());
        return OR_Failure;
    }

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult LibLoader::open(const std::string& path)
{
    void* p = dlopen(path.c_str(), RTLD_LOCAL | RTLD_NOW);

    if (p == nullptr)
    {
        // error
        clogError("Error while loading plugin %s: %s", path.c_str(), dlerror());
        return OR_Failure;
    }

    m_handle = p;
    return OR_OK;
}
//=============================================================================
bool LibLoader::isLoaded(const std::string& path) const
{
    void* p = dlopen(path.c_str(), RTLD_NOLOAD | RTLD_LOCAL | RTLD_NOW);

    if (p)
    {
        close(p);
        return true;
    }

    return false;
}
//=============================================================================
LibLoader::LibLoader()
    : m_handle(nullptr),
      m_noreg(false),
      m_path()
{
}
//=============================================================================
cerberus::OperationResult LibLoader::load(const std::string& path, bool noreg)
{
    OperationResult ret;
    void* p;

    if (m_handle && m_noreg)  // handle the current loaded library (unload)
    {
        // The referenced library image is not in the Cerberus register

        ret = close(m_handle);

        if (ret.fail())
        {
            return ret;
        }
    }

    m_handle = nullptr;
    m_noreg  = noreg;
    m_path   = path;

    if (noreg)
    {
        ret = open(path);

        if (ret.fail())
        {
            return ret;
        }

        return OR_OK;
    }

    ret = open(path);

    if (ret.fail())
    {
        return ret;
    }

    cdebug("loaded plugin %s in process memory", path.c_str());

    if (!core::CerberusRegister::addPlugin(m_handle, path))
    {
        // it already exists
        ret = close(m_handle);

        if (ret.fail())
        {
            return ret;
        }
    }

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult LibLoader::unload()
{
    if (!m_noreg) return OR_Unavailable;
    if (!m_handle) return OR_BadConditions;

    return close(m_handle);
}
//=============================================================================
cerberus::LoaderFunc LibLoader::get(const std::string& symbol)
{
    mutex::MutexLocker mut;

    if (!m_noreg)
    {
        mut = core::CerberusRegister::getPluginMutex(m_handle);
        if (!mut.isValid())
        {
            return {nullptr, mut};
        };
    }

    void* p = dlsym(m_handle, symbol.c_str());

    if (!p) cdebug("Error while searching symbol %s: %s", symbol.c_str(), dlerror());

    return {p, mut};
}
//=============================================================================
bool LibLoader::isLoaded() const { return isLoaded(m_path); }
//=============================================================================
cerberus::OperationResult LibLoader::fastload(const std::string& path)
{
    LibLoader loader;
    return loader.load(path);
}
//=============================================================================
