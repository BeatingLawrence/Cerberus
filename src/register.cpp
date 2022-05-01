#include "register.h"
#include "./exception/exceptioncatalog.h"
#include "./mutex/mutexlocker.h"

using namespace cerberus;

//=============================================================================
uint32_t Register::_findAvailableId_messageTemplates()  // TODO to optimize
{
    if(m_messageTemplates.empty())
    {
        return 1;
    }

    uint32_t id = m_messageTemplates.front().first;

    while(true)
    {
        bool finalIteration = true;

        for(auto& el : m_messageTemplates)
        {
            if(el.first == id)
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
    std::pair<uint32_t, message::MessageTemplate> pair(_findAvailableId_messageTemplates(), toAdd);
    m_messageTemplates.push_back(pair);
    return pair.first;
}
//=============================================================================
void Register::removeMessageTemplate(uint32_t idToRemove)
{
    mutex::MutexLocker locker(&m_messageTemplateMutex);

    for(auto it = m_messageTemplates.begin(); it != m_messageTemplates.end(); it++)
    {
        if((*it).first == idToRemove)
        {
            m_messageTemplates.erase(it);
            return;
        }
    }

    throw cerberusIllegalArgumentExc("Given ID does not exist");
}
//=============================================================================
uint32_t Register::messageIdByName(const std::string& name) const
{
    mutex::MutexLocker locker(&m_messageTemplateMutex);

    for(auto& el : m_messageTemplates)
    {
        if(el.second.name().compare(name) == 0)
        {
            return el.first;
        }
    }

    return Invalid_ID;
}
//=============================================================================
message::MessageTemplate Register::messageTemplateById(uint32_t id) const
{
    mutex::MutexLocker locker(&m_messageTemplateMutex);

    for(auto& el : m_messageTemplates)
    {
        if(el.first == id)
        {
            return el.second;
        }
    }

    throw cerberusIllegalArgumentExc("Given ID does not exist");
}
//=============================================================================
bool Register::messageTemplateNameAlreadyExists(const std::string& name) const
{
    mutex::MutexLocker locker(&m_messageTemplateMutex);

    for(auto& el : m_messageTemplates)
    {
        if(el.second.name().compare(name) == 0)
        {
            return true;
        }
    }

    return false;
}
//=============================================================================
