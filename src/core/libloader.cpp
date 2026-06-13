#include "libloader.h"

#ifdef WINDOWS_SYSTEM
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "../cerberus.h"

using namespace crb::core;

//=============================================================================
crb::OpRes LibLoader::close(void* handle)
{
#ifdef WINDOWS_SYSTEM
    if (FreeLibrary(static_cast<HMODULE>(handle)) == 0)
    {
        logError("Error while calling FreeLibrary: %lu", GetLastError());
        return OR_Failure;
    }
#else
    if (dlclose(handle) != 0)
    {
        // error
        logError("Error while calling dlclose: %s", dlerror());
        return OR_Failure;
    }
#endif

    return OR_OK;
}
//=============================================================================
crb::OpRes LibLoader::open(const std::string& path)
{
#ifdef WINDOWS_SYSTEM
    void* p = LoadLibraryA(path.c_str());
#else
    void* p = dlopen(path.c_str(), RTLD_LOCAL | RTLD_NOW);
#endif

    if (p == nullptr)
    {
        // error
#ifdef WINDOWS_SYSTEM
        logError("Error while loading plugin %s: %lu", path.c_str(), GetLastError());
#else
        logError("Error while loading plugin %s: %s", path.c_str(), dlerror());
#endif
        return OR_Failure;
    }

    m_handle = p;
    return OR_OK;
}
//=============================================================================
bool LibLoader::isLoaded(const std::string& path) const
{
#ifdef WINDOWS_SYSTEM
    HMODULE p = GetModuleHandleA(path.c_str());
#else
    void* p = dlopen(path.c_str(), RTLD_NOLOAD | RTLD_LOCAL | RTLD_NOW);
#endif

    if (p)
    {
#ifndef WINDOWS_SYSTEM
        close(p);
#endif
        return true;
    }

    return false;
}
//=============================================================================
LibLoader::LibLoader()
    : m_id(0),
      m_handle(nullptr),
      m_noreg(false),
      m_path()
{
}
//=============================================================================
LibLoader::~LibLoader() { unload(); }
//=============================================================================
crb::OpRes LibLoader::load(const std::string& path, bool noreg)
{
    OpRes ret;

    if (m_handle && m_noreg)  // handle the current loaded library (unload)
    {
        // The referenced library image is not in the Cerberus register

        ret = close(m_handle);

        if (ret.fail()) return ret;
    }

    m_handle = nullptr;
    m_noreg  = noreg;
    m_path   = path;

    if (noreg)
    {
        ret = open(path);

        if (ret.fail()) return ret;

        return OR_OK;
    }

    ret = open(path);

    if (ret.fail()) return ret;

    logDebug("loaded plugin %s in process memory", path.c_str());

    bool exists = false;
    m_id        = crb::Cerberus::addPlugin(m_handle, path, exists);

    if (exists)
    {
        // it already exists
        ret = close(m_handle);

        if (ret.fail()) return ret;
    }

    return OR_OK;
}
//=============================================================================
crb::OpRes LibLoader::unload()
{
    if (!m_noreg) return OR_Unavailable;
    if (!m_handle) return OR_BadConditions;

    auto ret = close(m_handle);

    if (ret.ok())
    {
        m_handle = nullptr;
        m_path   = "";
        return OR_OK;
    }

    return OR_Failure;
}
//=============================================================================
crb::OpRes LibLoader::swap(const std::string& path)
{
    if (m_noreg) return OR_Unavailable;

    auto ml = Cerberus::getPluginMutex(m_id);

    if (!ml.isValid()) return OR_NotFound;

    if (m_handle != Cerberus::checkPlugin(m_id)) return OR_NotFound;

    OpRes ret;

    ret = close(m_handle);

    if (ret.fail()) return ret;

    ret = open(path);

    if (ret.fail()) return ret;

    logDebug("swapped plugin %s[old] with %s[new]", m_path.c_str(), path.c_str());
    m_path = path;

    if (!Cerberus::updatePlugin(m_id, path, m_handle)) return OR_Failure;

    return OR_OK;
}
//=============================================================================
crb::OpRes LibLoader::reload() { return swap(m_path); }
//=============================================================================
crb::LoaderFunc LibLoader::get(const std::string& symbol)
{
    MutexLocker ml;

    if (!m_noreg)
    {
        ml = Cerberus::getPluginMutex(m_id);

        if (!ml.isValid()) return {nullptr, MutexLocker()};
    }

#ifdef WINDOWS_SYSTEM
    void* p = reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(m_handle), symbol.c_str()));
#else
    void* p = dlsym(m_handle, symbol.c_str());
#endif

#ifdef WINDOWS_SYSTEM
    if (!p) logDebug("Error while searching symbol %s: %lu", symbol.c_str(), GetLastError());
#else
    if (!p) logDebug("Error while searching symbol %s: %s", symbol.c_str(), dlerror());
#endif

    return {p, ml};
}
//=============================================================================
bool LibLoader::isLoaded() const { return isLoaded(m_path); }
//=============================================================================
crb::OpRes LibLoader::fastload(const std::string& path)
{
    LibLoader loader;
    return loader.load(path);
}
//=============================================================================
