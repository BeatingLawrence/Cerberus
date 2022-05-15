#include "register.h"
#include "./exception/exceptioncatalog.h"
#include "./mutex/mutexlocker.h"
#include "./cerberus.h"

using namespace cerberus;

//=============================================================================
uint32_t Register::_findAvailableTypeID_messageTemplates()  // TODO to optimize
{
    if(m_messageTemplates.empty())
    {
        return 1;
    }

    uint32_t id = m_messageTemplates.front().typeID;

    while(true)
    {
        bool finalIteration = true;

        for(auto& el : m_messageTemplates)
        {
            if(el.typeID == id)
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
uint32_t Register::_findAvailableID_threads()   // TODO to optimize
{
    if(m_threads.empty())
    {
        return 1;
    }

    uint32_t id = m_threads.front().threadID;

    while(true)
    {
        bool finalIteration = true;

        for(auto& el : m_threads)
        {
            if(el.threadID == id)
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
uint32_t Register::addMessageTemplate(const message::MessageTemplate& toAdd)
{
    mutex::MutexLocker locker(&m_messageTemplateMutex);
    MessageTemplateEntry entry = {_findAvailableTypeID_messageTemplates(), toAdd};
    m_messageTemplates.push_back(entry);
    return entry.typeID;
}
//=============================================================================
void Register::removeMessageTemplate(uint32_t id)
{
    mutex::MutexLocker locker(&m_messageTemplateMutex);

    for(auto it = m_messageTemplates.begin(); it != m_messageTemplates.end(); it++)
    {
        if((*it).typeID == id)
        {
            m_messageTemplates.erase(it);
            return;
        }
    }

    throw cerberusIllegalArgumentExc("Given Message template ID does not exist");
}
//=============================================================================
uint32_t Register::messageTypeIdByName(const std::string& name) const
{
    mutex::MutexLocker locker(&m_messageTemplateMutex);

    for(auto& el : m_messageTemplates)
    {
        if(el.tmplate.name().compare(name) == 0)
        {
            return el.typeID;
        }
    }

    return CERBERUS_INVALID_ID;
}
//=============================================================================
message::MessageTemplate Register::messageTemplateByTypeId(uint32_t id) const
{
    mutex::MutexLocker locker(&m_messageTemplateMutex);

    for(auto& el : m_messageTemplates)
    {
        if(el.typeID == id)
        {
            return el.tmplate;
        }
    }

    throw cerberusIllegalArgumentExc("Given Message template ID does not exist");
}
//=============================================================================
bool Register::messageTemplateNameAlreadyExists(const std::string& name) const
{
    mutex::MutexLocker locker(&m_messageTemplateMutex);

    for(auto& el : m_messageTemplates)
    {
        if(el.tmplate.name().compare(name) == 0)
        {
            return true;
        }
    }

    return false;
}
//=============================================================================
uint32_t Register::addThread(thread::Thread* thread, const std::string& name)
{
    mutex::MutexLocker locker(&m_threadMutex);
    ThreadEntry entry = {_findAvailableID_threads(), name, thread};
    m_threads.push_back(entry);
    return entry.threadID;
}
//=============================================================================
void Register::removeThread(uint32_t id)
{
    mutex::MutexLocker locker(&m_threadMutex);

    for(auto it = m_threads.begin(); it != m_threads.end(); it++)
    {
        if((*it).threadID == id)
        {
            logInfo(Cerberus::strPrint("Removed Thread '%s' with ID: %u", (*it).name.c_str(), (*it).threadID));
            m_threads.erase(it);
            return;
        }
    }

    throw cerberusIllegalArgumentExc("Given Thread ID does not exist");
}
//=============================================================================
uint32_t Register::threadIdByName(const std::string& name) const
{
    mutex::MutexLocker locker(&m_threadMutex);

    for(auto& el : m_threads)
    {
        if(el.name.compare(name) == 0)
        {
            return el.threadID;
        }
    }

    return CERBERUS_INVALID_ID;
}
//=============================================================================
thread::Thread* Register::threadById(uint32_t id) const
{
    mutex::MutexLocker locker(&m_threadMutex);

    for(auto& el : m_threads)
    {
        if(el.threadID == id)
        {
            return el.thread;
        }
    }

    throw cerberusIllegalArgumentExc("Given Thread ID does not exist");
}
//=============================================================================
bool Register::threadNameAlreadyExists(const std::string& name) const
{
    mutex::MutexLocker locker(&m_threadMutex);

    for(auto& el : m_threads)
    {
        if(el.name.compare(name) == 0)
        {
            return true;
        }
    }

    return false;
}
//=============================================================================
uint32_t Register::registerCerberusObject(CerberusObject* object)
{
    mutex::MutexLocker locker(&m_objectMutex);

    for(auto& el : m_objects)
    {
        if(el->name().compare(object->name()) == 0)
        {
            throw cerberusIllegalArgumentExc("Cannot register a duplicate name");
        }
    }

    cerberus_object newObject(object);
    m_objects.push_back(newObject);
    return _findAvailableID_objects();
}
//=============================================================================
uint32_t Register::cerberusObjectByName(const std::string& name) const
{
    mutex::MutexLocker locker(&m_objectMutex);

    for(auto& el : m_objects)
    {
        if(el->name().compare(name) == 0)
        {
            return el->id();
        }
    }

    return CERBERUS_INVALID_ID;
}
//=============================================================================
