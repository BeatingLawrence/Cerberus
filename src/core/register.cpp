#include "register.h"

#include <cstdlib>

#include "cerberusobject.h"
#include "cerberusutils.h"
#include "src/define.h"
#include "src/mutex/mutexlocker.h"

using namespace cerberus;

//=============================================================================
uint32_t Register::findAvailableId()
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
Register::Register() { srand(584239578); }
//=============================================================================
Register::~Register() {}
//=============================================================================
uint32_t Register::registerObject(CerberusObject* object)
{
    if (object == nullptr)
    {
        return CERBERUS_INVALID_ID;
    }

    mutex::MutexLocker locker(&registerMutex);

    if (!object->name().empty())
    {
        for (auto&& el : m_objects)
        {
            if (core::CerberusUtils::areEqual(object->name(), el->name()))
            {
                return CERBERUS_INVALID_ID;
            }
        }
    }

    uint32_t id = findAvailableId();
    m_objects.push_back(object);
    return id;
}
//=============================================================================
void Register::unregisterObject(uint32_t id)
{
    if (id == CERBERUS_INVALID_ID) return;

    mutex::MutexLocker locker(&registerMutex);

    for (auto it = m_objects.begin(); it != m_objects.end(); it++)
    {
        if ((*it)->id() == id)
        {
            debug("Unregistering %s", (*it)->toObjStr().c_str());
            m_objects.erase(it);
            return;
        }
    }
}
//=============================================================================
CerberusObject* Register::objectByName(const std::string& name)
{
    if (name.empty())
    {
        return nullptr;
    }

    mutex::MutexLocker locker(&registerMutex);

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
CerberusObject* Register::objectById(uint32_t id)
{
    if (id == CERBERUS_INVALID_ID)
    {
        return nullptr;
    }

    mutex::MutexLocker locker(&registerMutex);

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
