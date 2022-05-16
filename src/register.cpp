#include "register.h"
#include "./exception/exceptioncatalog.h"
#include "./mutex/mutexlocker.h"
#include "./cerberusobject.h"
#include "./cerberus.h"

using namespace cerberus;

//=============================================================================
uint32_t Register::_findAvailableID_objects()
{
    if(m_objects.empty())
    {
        return 1;
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

    m_objects.push_back(object);
    return _findAvailableID_objects();
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
                logInfo(cerberus::Cerberus::strPrint("Unregistering object with ID: %u", id));
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
bool Register::isEmpty() const
{
    return m_objects.empty();
}
//=============================================================================
void Register::freeMemory()
{
    std::list<CerberusObject*> deleteList;

    for(auto it = m_objects.begin(); it != m_objects.end(); it++)
    {
        if(*it != nullptr)
        {
            if((*it)->type() != CERBERUS_OBJECT_THREAD)
            {
                deleteList.push_back(*it);
            }
        }
    }

    for(auto& el : deleteList)
    {
        logInfo(cerberus::Cerberus::strPrint("Deleting ID: %u", el->id()));
        delete el;
    }
}
//=============================================================================
