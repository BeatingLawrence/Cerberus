#include "cerberusregister.h"

#include <dlfcn.h>

#include <cstdlib>

#include "../cerberus.h"
#include "../define.h"
#include "../mutex/mutexlocker.h"
#include "../thread/thread.h"
#include "cerberusobject.h"

using namespace cerberus;
using namespace cerberus::core;
using namespace cerberus::message;

//=============================================================================
uint32_t CerberusRegister::findAvailableId()
{
    while (true)
    {
        uint32_t id = (uint32_t)rand();

        bool found = false;
        for (auto&& el : m_objects)
        {
            if (id == el->id())
            {
                found = true;
                break;
            }
        }

        if (id < CERBERUS_FACTORY_START_ID)
        {
            continue;
        }

        if (!found) return id;
    }
}
//=============================================================================
uint32_t CerberusRegister::findAvailablePluginId()
{
    while (true)
    {
        uint32_t id = (uint32_t)rand();

        bool found = false;
        for (auto&& el : m_plugins)
        {
            if (id == el.id)
            {
                found = true;
                break;
            }
        }

        if (!found) return id;
    }
}
//=============================================================================
CerberusRegister::CerberusRegister() { srand(584239578); }
//=============================================================================
void CerberusRegister::registerObj(CerberusObject* object)
{
    if (object == nullptr) return;

    mutex::MutexLocker locker(m_mutex);

    if (!object->name().empty())
    {
        for (auto& el : m_objects)
        {
            if (CerberusUtils::areEqual(object->name(), el->name())) return;
        }
    }

    object->m_id = findAvailableId();

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
void CerberusRegister::unregisterObj(uint32_t id)
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
uint32_t CerberusRegister::addPlugin(void* handle, const std::string& path, bool& exists)
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

    uint32_t id = findAvailablePluginId();

    m_plugins.push_back(std::move(Plugin(id, handle, path)));

    return id;
}
//=============================================================================
void CerberusRegister::removePlugin(uint32_t id)
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
void* CerberusRegister::checkPlugin(uint32_t id)
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
mutex::MutexLocker CerberusRegister::getPluginMutex(uint32_t id)
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
CerberusObject* CerberusRegister::objById(uint32_t id)
{
    if (id == CERBERUS_INVALID_ID) return nullptr;

    for (auto&& el : m_objects)
    {
        if (el->id() == id)
        {
            return el;
        }
    }

    return nullptr;
}
//=============================================================================
uint32_t CerberusRegister::objIdByName(const std::string& name)
{
    mutex::MutexLocker locker(m_mutex);

    CerberusObject* found = objByName(name);

    if (found == nullptr) return CERBERUS_INVALID_ID;

    return found->id();
}
//=============================================================================
MessageTemplate CerberusRegister::msgTemplateByName(const std::string& name)
{
    mutex::MutexLocker locker(m_mutex);

    CerberusObject* found = objByName(name);

    if (found == nullptr)
    {
        return message::MessageTemplate();
    }
    else
    {
        if (found->type() == CerberusObject::ObjectType::MessageTemplate)
        {
            return *(found->to_p<message::MessageTemplate>());
        }
        else
        {
            return message::MessageTemplate();
        }
    }
}
//=============================================================================
MessageTemplate CerberusRegister::msgTemplateById(uint32_t id)
{
    mutex::MutexLocker locker(m_mutex);
    CerberusObject* found = objById(id);

    if (found == nullptr)
    {
        return message::MessageTemplate();
    }
    else
    {
        if (found->type() == CerberusObject::ObjectType::MessageTemplate)
        {
            return message::MessageTemplate(*(found->to_p<message::MessageTemplate>()));
        }
        else
        {
            return message::MessageTemplate();
        }
    }
}
//=============================================================================
void CerberusRegister::sendMsgToObj(uint32_t id, cerberus_message msg)
{
    mutex::MutexLocker locker(m_mutex);
    CerberusObject* found = objById(id);

    if (found->type() == CerberusObject::ObjectType::Thread)
    {
        auto thr = found->to_p<cerberus::thread::Thread>();
        thr->addMessage(msg);
    }
}
//=============================================================================
CerberusObject* CerberusRegister::objByName(const std::string& name)
{
    if (name.empty())
    {
        return nullptr;
    }

    for (auto&& el : m_objects)
    {
        if (core::CerberusUtils::areEqual(el->name(), name))
        {
            return el;
        }
    }

    return nullptr;
}
//=============================================================================
