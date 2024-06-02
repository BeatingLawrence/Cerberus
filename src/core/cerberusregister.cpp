#include "cerberusregister.h"

#include <dlfcn.h>

#include "../cerberus.h"
#include "../define.h"
#include "../exception/exception.h"
#include "../thread/mutexlocker.h"
#include "recordable.h"

#define hashFunc(str) CerberusUtils::hash_fnv1a(str)
#define hashFunc_res(str) CerberusRegister::removeReserved(hashFunc(str))

using namespace cerberus;
using namespace cerberus::core;

//=============================================================================
HASH32 CerberusRegister::removeReserved(HASH32 hash) { return hash & 0xffffff00; }
//=============================================================================
HASH32 CerberusRegister::findAvailableObjId(const std::string& name)
{
    HASH32 h = hashFunc_res(name);

    for (auto&& el : m_objects)  // collision detection
        if (h == el->id()) throw cFatalExc("collision encountered while hashing %s", name.c_str());

    return h;
}
//=============================================================================
HASH32 CerberusRegister::findAvailableTmpltId(const std::string& name)
{
    HASH32 h = hashFunc_res(name);

    for (auto&& el : m_templates)  // collision detection
        if (h == el.id) throw cFatalExc("collision encountered while hashing %s", name.c_str());

    return h;
}
//=============================================================================
HASH32 CerberusRegister::findAvailablePluginId(const std::string& path)
{
    HASH32 h = hashFunc(path);

    for (auto&& el : m_plugins)  // collision detection
        if (h == el.id) throw cFatalExc("collision encountered while hashing %s", path.c_str());

    return h;
}
//=============================================================================
OpResData<Recordable*> CerberusRegister::objById(HASH32 id)
{
    if (id == CERBERUS_INVALID_ID) return OR_WrongArgument;

    for (auto&& el : m_objects)
        if (el->m_id == id) return el;

    return OR_NotFound;
}
//=============================================================================
OpResData<Recordable*> CerberusRegister::objByName(const std::string& name)
{
    if (name.empty()) return OR_WrongArgument;

    return objById(hashFunc_res(name));
}
//=============================================================================
void CerberusRegister::cleanup() { m_objects.clear(); }
//=============================================================================
CerberusRegister::CerberusRegister() {}
//=============================================================================
CerberusRegister::~CerberusRegister() { cleanup(); }
//=============================================================================
OpResData<MessageTemplate> CerberusRegister::msgTemplateByName(const std::string& name)
{
    return msgTemplateById(hashFunc_res(name));
}
//=============================================================================
OpResData<MessageTemplate> CerberusRegister::msgTemplateById(HASH32 id)
{
    if (id == CERBERUS_INVALID_ID) return OR_WrongArgument;

    MutexLocker locker(m_tmpltMutex);

    for (auto&& el : m_templates)
    {
        if (el.id == id) return el;
    }

    return OR_NotFound;
}
//=============================================================================
OpResData<HASH32> CerberusRegister::addMsgTemplate(const MessageTemplate& tmplt)
{
    if (tmplt.name().empty()) return OR_WrongArgument;

    MutexLocker locker(m_tmpltMutex);

    MessageTemplate t(tmplt);
    t.id = findAvailableTmpltId(tmplt.name());
    m_templates.push_back(t);

    return t.id;
}
//=============================================================================
void CerberusRegister::registerObj(Recordable* object)
{
    if (object == nullptr) return;

    if (object->name().empty()) throw cIllegalArgExc("Registering objects must have a name");

    MutexLocker locker(m_objMutex);

    object->m_id = findAvailableObjId(object->name());

    switch (object->type())
    {
        case Recordable::COBJ_Thread:
            m_objects.push_back(object);
            break;

        default:
            throw cIllegalArgExc("unknown type");
    }

    logDebug("New %s", object->toObjStr().c_str());
}
//=============================================================================
void CerberusRegister::unregisterObj(HASH32 id)
{
    if (id == CERBERUS_INVALID_ID) return;

    MutexLocker locker(m_objMutex);

    for (auto it = m_objects.begin(); it != m_objects.end(); it++)
    {
        if ((*it)->id() == id)
        {
            logDebug("Unregistering %s", (*it)->toObjStr().c_str());
            m_objects.erase(it);
            return;
        }
    }
}
//=============================================================================
HASH32 CerberusRegister::objIdByName(const std::string& name)
{
    MutexLocker locker(m_objMutex);
    auto found = objByName(name);

    if (found.fail()) return CERBERUS_INVALID_ID;

    return found.value->m_id;
}
//=============================================================================
void CerberusRegister::sendMsgToObj(HASH32 id, cerberus_message msg)
{
    MutexLocker locker(m_objMutex);
    auto found = objById(id);

    if (found.ok()) found.value->addMessage(msg);
}
//=============================================================================
void CerberusRegister::sendMsgToObj(const std::string& name, cerberus_message msg)
{
    MutexLocker locker(m_objMutex);
    auto found = objByName(name);

    if (found.ok()) found.value->addMessage(msg);
}
//=============================================================================
HASH32 CerberusRegister::addPlugin(void* handle, const std::string& path, bool& exists)
{
    MutexLocker locker(m_pluginMutex);

    // check if plugin exists

    for (auto&& el : m_plugins)
    {
        if (el.handle == handle)
        {
            exists = true;
            return el.id;
        }
    }

    exists = false;

    uint32_t id = findAvailablePluginId(path);

    if (id == CERBERUS_INVALID_ID) throw cIllegalArgExc("Plugin is a duplicate");

    m_plugins.push_back(std::move(Plugin(id, handle, path)));

    return id;
}
//=============================================================================
void CerberusRegister::removePlugin(HASH32 id)
{
    MutexLocker locker(m_pluginMutex);

    for (auto it = m_plugins.begin(); it != m_plugins.end(); it++)
    {
        if ((*it).id == id)
        {
            (*it).mutex.lock();  // wait the lock
            (*it).mutex.unlock();
            m_plugins.erase(it);
            return;
        }
    }
}
//=============================================================================
void CerberusRegister::cleanupPlugins()
{
    MutexLocker locker(m_pluginMutex);

    for (auto&& el : m_plugins)
    {
        el.mutex.lock();  // wait the lock
        el.mutex.unlock();
        int ret = dlclose(el.handle);

        if (ret != 0) logError("Error while unloading plugin %u %s", el.id, el.path.c_str());

        return;
    }

    m_plugins.clear();
}
//=============================================================================
void* CerberusRegister::checkPlugin(HASH32 id)
{
    MutexLocker locker(m_pluginMutex);

    for (auto&& el : m_plugins)
        if (el.id == id) return el.handle;

    return nullptr;
}
//=============================================================================
MutexLocker CerberusRegister::getPluginMutex(HASH32 id)
{
    MutexLocker locker(m_pluginMutex);

    for (auto&& el : m_plugins)
        if (el.id == id) return el.mutex;

    return MutexLocker();  // invalid
}
//=============================================================================
bool CerberusRegister::updatePlugin(uint32_t id, const std::string& path, void* handle)
{
    MutexLocker locker(m_pluginMutex);

    for (auto&& el : m_plugins)
    {
        if (el.id == id)
        {
            el.handle = handle;
            el.path   = path;
            return true;
        }
    }

    return false;
}
//=============================================================================
