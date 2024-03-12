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

#define FNV_OFFSET_BASIS 0x811c9dc5
#define FNV_PRIME 0x01000193

//=============================================================================
HASH32 CerberusRegister::hash(const std::string& str)
{
    // This method uses FNV1A algorithm
    HASH32 hash    = FNV_OFFSET_BASIS;
    const char* ch = str.c_str();

    while ((*ch) != '\0')
    {
        hash = hash ^ (HASH32)(*ch);
        hash = hash * FNV_PRIME;
        ch++;
    }

    return hash;
}
//=============================================================================
HASH32 CerberusRegister::removeReserved(HASH32 hash) { return hash & 0xffffff00; }
//=============================================================================
HASH32 CerberusRegister::findAvailableObjectId(const std::string& name)
{
    HASH32 h = removeReserved(hash(name));

    for (auto&& el : m_objects)
        if (h == el->id()) return CERBERUS_INVALID_ID;

    return h;
}
//=============================================================================
HASH32 CerberusRegister::findAvailablePluginId(const std::string& path)
{
    HASH32 h = hash(path);

    for (auto&& el : m_plugins)
        if (h == el.id) return CERBERUS_INVALID_ID;

    return h;
}
//=============================================================================
CerberusRegister::CerberusRegister() {}
//=============================================================================
CerberusRegister::~CerberusRegister()
{
    // destroy all cerberus-managed objects (none, at the moment)
}
//=============================================================================
void CerberusRegister::registerObj(CerberusObject* object)
{
    if (object == nullptr) return;

    if (object->name().empty()) throw cerberusIllegalArgExc("Registering objects must be named objects");

    mutex::MutexLocker locker(m_mutex);

    object->m_id = findAvailableObjectId(object->name());

    if (object->m_id == CERBERUS_INVALID_ID) throw cerberusIllegalArgExc("Object is a duplicate");

    switch (object->type())
    {
        case CerberusObject::Thread:
        case CerberusObject::Socket:
            m_objects.push_back(object);
            break;

        case CerberusObject::MessageTemplate:
        {
            auto mt = new MessageTemplate(*(object->to_p<MessageTemplate>()));  // copy
            m_objects.push_back(mt);
        }
        break;

        default:
            throw cerberusIllegalArgExc("given object has no type");
    }

    logDebug("New %s", object->toObjStr().c_str());
}
//=============================================================================
void CerberusRegister::unregisterObj(HASH32 id)
{
    if (id == CERBERUS_INVALID_ID) return;

    mutex::MutexLocker locker(m_mutex);

    for (auto it = m_objects.begin(); it != m_objects.end(); it++)
    {
        if ((*it)->id() == id)
        {
            logDebug("Unregistering %s", (*it)->toObjStr().c_str());

            if ((*it)->type() == CerberusObject::MessageTemplate)
            {
                delete *it;
            }

            m_objects.erase(it);

            return;
        }
    }
}
//=============================================================================
HASH32 CerberusRegister::addPlugin(void* handle, const std::string& path, bool& exists)
{
    mutex::MutexLocker locker(m_mutex);

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
    mutex::MutexLocker locker(m_mutex);

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
    mutex::MutexLocker locker(m_mutex);

    for (auto&& el : m_plugins)
    {
        el.mutex.lock();  // wait the lock
        el.mutex.unlock();
        int ret = dlclose(el.handle);
        if (ret != 0)
        {
            logError("Error while unloading plugin %u %s", el.id, el.path.c_str());
        }

        return;
    }

    m_plugins.clear();
}
//=============================================================================
void* CerberusRegister::checkPlugin(HASH32 id)
{
    mutex::MutexLocker locker(m_mutex);

    for (auto&& el : m_plugins)
    {
        if (el.id == id)
        {
            return el.handle;
        }
    }

    return nullptr;
}
//=============================================================================
mutex::MutexLocker CerberusRegister::getPluginMutex(HASH32 id)
{
    mutex::MutexLocker locker(m_mutex);

    for (auto&& el : m_plugins)
    {
        if (el.id == id)
        {
            return el.mutex;
        }
    }

    return mutex::MutexLocker();  // invalid
}
//=============================================================================
bool CerberusRegister::updatePlugin(uint32_t id, const std::string& path, void* handle)
{
    mutex::MutexLocker locker(m_mutex);

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
    {
        if (el->id() == id)
        {
            return el;
        }
    }

    return OR_NotFound;
}
//=============================================================================
HASH32 CerberusRegister::objIdByName(const std::string& name)
{
    mutex::MutexLocker locker(m_mutex);

    auto found = objByName(name);

    if (found.fail()) return CERBERUS_INVALID_ID;

    return found.value->id();
}
//=============================================================================
MessageTemplate CerberusRegister::msgTemplateByName(const std::string& name)
{
    mutex::MutexLocker locker(m_mutex);

    auto found = objByName(name).expect();

    if (found.value->type() == CerberusObject::ObjectType::MessageTemplate)
    {
        return *(found.value->to_p<message::MessageTemplate>());
    }
    else
    {
        return message::MessageTemplate();
    }
}
//=============================================================================
MessageTemplate CerberusRegister::msgTemplateById(HASH32 id)
{
    if (id == CERBERUS_INVALID_ID) throw cerberusIllegalArgExc("invalid id");

    if (id < 256u)  // reserved range
    {
        return standardTemplate(id);
    }

    mutex::MutexLocker locker(m_mutex);

    auto found = objById(id).expect();

    if (found.value->type() == CerberusObject::ObjectType::MessageTemplate)
    {
        return *(found.value->to_p<message::MessageTemplate>());
    }
    else
    {
        return message::MessageTemplate();
    }
}
//=============================================================================
void CerberusRegister::sendMsgToObj(HASH32 id, cerberus_message msg)
{
    mutex::MutexLocker locker(m_mutex);
    auto found = objById(id).expect();

    if (found.value->type() == CerberusObject::ObjectType::Thread)
    {
        auto thr = found.value->to_p<cerberus::thread::Thread>();
        thr->addMessage(msg);
    }
}
//=============================================================================
OpResData<CerberusObject*> CerberusRegister::objByName(const std::string& name)
{
    if (name.empty()) return OR_WrongArgument;

    for (auto&& el : m_objects)
    {
        if (core::CerberusUtils::areEqual(el->name(), name))
        {
            return el;
        }
    }

    return OR_NotFound;
}
//=============================================================================
MessageTemplate CerberusRegister::standardTemplate(HASH32 id)
{
    MessageTemplate tmplt;

    switch (id)
    {
        case CERBERUS_MESSAGE_LOG_ID:
            tmplt.addSlotType(ST_STRING);
            break;

        case CERBERUS_MESSAGE_TERM_ID:
            // nothing to add
            break;

            // add here more message specializations..

        default:
            throw cerberusImplMissExc("Requested standard message is not defined");
            break;
    }

    return tmplt;
}
//=============================================================================
