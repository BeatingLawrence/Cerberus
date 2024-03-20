#include "cerberusregister.h"

#include <dlfcn.h>

#include "../cerberus.h"
#include "../define.h"
#include "../mutex/mutexlocker.h"
#include "../thread/thread.h"
#include "cerberusobject.h"

using namespace cerberus;
using namespace cerberus::core;
using namespace cerberus::message;

//=============================================================================
HASH32 CerberusRegister::removeReserved(HASH32 hash) { return hash & 0xffffff00; }
//=============================================================================
HASH32 CerberusRegister::findAvailableObjectId(const std::string& name)
{
    HASH32 h = removeReserved(CerberusUtils::hash_fnv1a(name));

    for (auto&& el : m_objects)
        if (h == el->id()) return CERBERUS_INVALID_ID;

    return h;
}
//=============================================================================
HASH32 CerberusRegister::findAvailablePluginId(const std::string& path)
{
    HASH32 h = CerberusUtils::hash_fnv1a(path);

    for (auto&& el : m_plugins)
        if (h == el.id) return CERBERUS_INVALID_ID;

    return h;
}
//=============================================================================
CerberusRegister::CerberusRegister() {}
//=============================================================================
CerberusRegister::~CerberusRegister() { cleanup(); }
//=============================================================================
void CerberusRegister::registerObj(CerberusObject* object)
{
    if (object == nullptr) return;

    if (object->name().empty()) throw cerberusIllegalArgExc("Registering objects must be named objects");

    MutexLocker locker(m_mutex);

    object->m_id = findAvailableObjectId(object->name());

    if (object->m_id == CERBERUS_INVALID_ID) throw cerberusIllegalArgExc("Object is a duplicate");

    switch (object->type())
    {
        case CerberusObject::COBJ_Thread:
        case CerberusObject::COBJ_Socket:
            m_objects.push_back(object);
            break;

        case CerberusObject::COBJ_MessageTmplt:
        {
            auto mt = new MessageTemplate(*(object->to_p<MessageTemplate>()));  // copy
            m_objects.push_back(mt);
        }
        break;

        default:
            throw cerberusIllegalArgExc("unknown type");
    }

    logDebug("New %s", object->toObjStr().c_str());
}
//=============================================================================
void CerberusRegister::unregisterObj(HASH32 id)
{
    if (id == CERBERUS_INVALID_ID) return;

    MutexLocker locker(m_mutex);

    for (auto it = m_objects.begin(); it != m_objects.end(); it++)
    {
        if ((*it)->id() == id)
        {
            logDebug("Unregistering %s", (*it)->toObjStr().c_str());

            if ((*it)->type() == CerberusObject::COBJ_MessageTmplt) delete *it;

            m_objects.erase(it);

            return;
        }
    }
}
//=============================================================================
HASH32 CerberusRegister::addPlugin(void* handle, const std::string& path, bool& exists)
{
    MutexLocker locker(m_mutex);

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

    if (id == CERBERUS_INVALID_ID) throw cerberusIllegalArgExc("Plugin is a duplicate");

    m_plugins.push_back(std::move(Plugin(id, handle, path)));

    return id;
}
//=============================================================================
void CerberusRegister::removePlugin(HASH32 id)
{
    MutexLocker locker(m_mutex);

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
    MutexLocker locker(m_mutex);

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
    MutexLocker locker(m_mutex);

    for (auto&& el : m_plugins)
        if (el.id == id) return el.handle;

    return nullptr;
}
//=============================================================================
MutexLocker CerberusRegister::getPluginMutex(HASH32 id)
{
    MutexLocker locker(m_mutex);

    for (auto&& el : m_plugins)
        if (el.id == id) return el.mutex;

    return MutexLocker();  // invalid
}
//=============================================================================
bool CerberusRegister::updatePlugin(uint32_t id, const std::string& path, void* handle)
{
    MutexLocker locker(m_mutex);

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
OpResData<CerberusObject*> CerberusRegister::objById(HASH32 id)
{
    if (id == CERBERUS_INVALID_ID) return OR_WrongArgument;

    for (auto&& el : m_objects)
        if (el->id() == id) return el;

    return OR_NotFound;
}
//=============================================================================
HASH32 CerberusRegister::objIdByName(const std::string& name)
{
    MutexLocker locker(m_mutex);

    auto found = objByName(name);

    if (found.fail()) return CERBERUS_INVALID_ID;

    return found.value->id();
}
//=============================================================================
MessageTemplate CerberusRegister::msgTemplateByName(const std::string& name)
{
    MutexLocker locker(m_mutex);

    auto found = objByName(name).expect();

    if (found.value->type() == CerberusObject::ObjectType::COBJ_MessageTmplt)
        return *(found.value->to_p<message::MessageTemplate>());
    else
        return message::MessageTemplate();
}
//=============================================================================
MessageTemplate CerberusRegister::msgTemplateById(HASH32 id)
{
    if (id == CERBERUS_INVALID_ID) throw cerberusIllegalArgExc("invalid id");

    if (id < 256u) throw cerberusIllegalArgExc("reserved range not accessible by register");

    MutexLocker locker(m_mutex);

    auto found = objById(id).expect();

    if (found.value->type() == CerberusObject::ObjectType::COBJ_MessageTmplt)
        return *(found.value->to_p<message::MessageTemplate>());
    else
        throw cerberusIllegalArgExc("requested template is not a template");
}
//=============================================================================
void CerberusRegister::sendMsgToObj(HASH32 id, cerberus_message msg)
{
    MutexLocker locker(m_mutex);
    auto found = objById(id).expect();

    if (found.value->type() == CerberusObject::ObjectType::COBJ_Thread)
    {
        auto thr = found.value->to_p<cerberus::Thread>();
        thr->addMessage(msg);
    }
}
//=============================================================================
OpResData<bool> CerberusRegister::isCerbManaged(HASH32 id)
{
    MutexLocker locker(m_mutex);
    auto obj = objById(id);

    if (obj.fail()) return obj;

    return obj.value->m_cerbManaged;
}
//=============================================================================
OpResData<CerberusObject*> CerberusRegister::objByName(const std::string& name)
{
    if (name.empty()) return OR_WrongArgument;

    HASH32 h = removeReserved(CerberusUtils::hash_fnv1a(name));

    for (auto&& el : m_objects)
        if (el->m_id == h) return el;

    return OR_NotFound;
}
//=============================================================================
void CerberusRegister::cleanup()
{
    for (auto&& el : m_objects)
    {
        if (el->m_cerbManaged)
        {
            if (el->m_type == CerberusObject::COBJ_Thread)
            {
                el->to_p<Thread>()->join(true);
                logDebug("Joined %s", el->name().c_str());
            }

            delete el;
            continue;
        }

        switch (el->m_type)
        {
            case CerberusObject::COBJ_MessageTmplt:
                delete el;
            default:
                break;
        }
    }

    m_objects.clear();
}
//=============================================================================
