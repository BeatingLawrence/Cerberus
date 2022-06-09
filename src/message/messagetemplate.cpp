#include "messagetemplate.h"
#include "./message.h"
#include "../core/cerberusutils.h"

using namespace cerberus::message;

//=============================================================================
MessageTemplate::MessageTemplate(const std::string& name) :
    CerberusObject(CerberusObject::OT_MessageTemplate, name)
{
    logInfo("New Message template '%s' with ID: %u", name.c_str(), id());
}
//=============================================================================
MessageTemplate::MessageTemplate(const Message& message, const std::string& name) :
    CerberusObject(CerberusObject::OT_MessageTemplate, name)
{
    logInfo("New Message template '%s' with ID: %u", name.c_str(), id());

    for(size_t i = 0; i < message.count(); i++)
    {
        m_types.push_back(message.getSlotAt(i)->type());
    }
}
//=============================================================================
MessageTemplate::~MessageTemplate()
{
    // noop
}
//=============================================================================
void MessageTemplate::addSlotType(slot::SlotType type)
{
    m_types.push_back(type);
}
//=============================================================================
slot::SlotType MessageTemplate::getSlotTypeAt(size_t index) const
{
    return m_types[index];
}
//=============================================================================
size_t MessageTemplate::count() const
{
    return m_types.size();
}
//=============================================================================
