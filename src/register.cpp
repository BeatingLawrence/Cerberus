#include "register.h"
#include "./exception/exceptioncatalog.h"
#include "./mutex/mutexlocker.h"
#include "./cerberusobject.h"
#include "./cerberus.h"
#include "./core/cerberusutils.h"

using namespace cerberus;

//=============================================================================
uint32_t Register::_findAvailableID_objects()
{
    if(m_objects.empty())
    {
        return CERBERUS_FACTORY_START_ID;
    }

    uint32_t id = m_objects.front()->id();

    while(true)
    {
        bool finalIteration = true;

        for(auto& el : m_objects)
        {
            if(el->id() == id)
            {
                id++;
                finalIteration = false;
                break;
            }
        }

        if(finalIteration)
        {
            return id;
        }
    }
}
//=============================================================================
Register::Register()
{
}
//=============================================================================
Register::~Register()
{
    mutex::MutexLocker locker(&m_objectMutex);
}
//=============================================================================
uint32_t Register::registerCerberusObject(CerberusObject* object)
{
    mutex::MutexLocker locker(&m_objectMutex);

    if(object == nullptr)
    {
        throw cerberusIllegalArgumentExc("Given object is null");
    }

    for(auto it = m_objects.begin(); it != m_objects.end(); it++)
    {
        if((*it) != nullptr)
        {
            if((*it)->name().compare(object->name()) == 0)
            {
                throw cerberusIllegalArgumentExc("Cannot register a duplicate name");
            }
        }
    }

    uint32_t id = _findAvailableID_objects();
    m_objects.push_back(object);
    return id;
}
//=============================================================================
void Register::unregisterCerberusObject(uint32_t id)
{
    mutex::MutexLocker locker(&m_objectMutex);

    if(id == CERBERUS_INVALID_ID)
    {
        throw cerberusIllegalArgumentExc("Given ID is invalid");
    }

    for(auto it = m_objects.begin(); it != m_objects.end(); it++)
    {
        if((*it) != nullptr)
        {
            if((*it)->id() == id)
            {
                logInfo("Unregistering object with ID: %u", id);
                m_objects.erase(it);
                return;
            }
        }
    }
}
//=============================================================================
CerberusObject* Register::cerberusObjectByName(const std::string& name) const
{
    mutex::MutexLocker locker(&m_objectMutex);

    for(auto it = m_objects.begin(); it != m_objects.end(); it++)
    {
        if((*it) != nullptr)
        {
            if((*it)->name().compare(name) == 0)
            {
                return (*it);
            }
        }
    }

    return nullptr;
}
//=============================================================================
CerberusObject* Register::cerberusObjectByID(uint32_t id) const
{
    mutex::MutexLocker locker(&m_objectMutex);

    for(auto it = m_objects.begin(); it != m_objects.end(); it++)
    {
        if((*it) != nullptr)
        {
            if((*it)->id() == id)
            {
                return (*it);
            }
        }
    }

    return nullptr;
}
//=============================================================================
void Register::freeMemory()
{
    if(m_objects.empty())
    {
        return;
    }

    logInfo("Trying to free Register memory..");
    std::list<CerberusObject*> deleteList;

    for(auto const& i : m_objects)
    {
        if(i != nullptr)
        {
            if(i->type() != CerberusObject::ObjectType::OT_Thread)
            {
                deleteList.push_back(i);
            }
        }
    }

    for(auto& el : deleteList)
    {
        logInfo("Deleting ID: %u of name: %s", el->id(), el->name().c_str());
        delete el;
    }
}
//=============================================================================
