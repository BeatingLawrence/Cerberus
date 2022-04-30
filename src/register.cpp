#include "register.h"
#include "./exception/exceptioncatalog.h"

using namespace cerberus;

//=============================================================================
uint32_t Register::_findAvailableId_messageTemplates()  // TODO to optimize
{
    if(m_messageTemplates.empty())
    {
        return 1;
    }

    uint32_t id = m_messageTemplates.front().id();

    while(true)
    {
        bool finalIteration = true;

        for(auto& el : m_messageTemplates)
        {
            if(el.id() == id)
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
    message::MessageTemplate msgtmp(toAdd);
    msgtmp.setId(_findAvailableId_messageTemplates());
    m_messageTemplates.push_back(msgtmp);
    return msgtmp.id();
}
//=============================================================================
void Register::removeMessageTemplate(uint32_t idToRemove)
{
    for(auto it = m_messageTemplates.begin(); it != m_messageTemplates.end(); it++)
    {
        if((*it).id() == idToRemove)
        {
            m_messageTemplates.erase(it);
            return;
        }
    }

    throw cerberusIllegalArgumentExc("Given ID does not exist");
}
//=============================================================================
message::MessageTemplate Register::messageTemplateById(uint32_t id) const
{
    for(auto& el : m_messageTemplates)
    {
        if(el.id() == id)
        {
            return el;
        }
    }

    throw cerberusIllegalArgumentExc("Given ID does not exist");
}
//=============================================================================
