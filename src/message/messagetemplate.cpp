#include "messagetemplate.h"

#include "./message.h"

using namespace cerberus::message;

//=============================================================================
MessageTemplate::MessageTemplate()
    : CerberusObject(CerberusObject::InvalidObject)
{
}
//=============================================================================
MessageTemplate::MessageTemplate(const std::string& name)
    : CerberusObject(CerberusObject::MessageTemplate, name)
{
}
//=============================================================================
MessageTemplate::MessageTemplate(const Message& message, const std::string& name)
    : CerberusObject(CerberusObject::MessageTemplate, name)
{
    for (size_t i = 0; i < message.count(); i++)
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
void MessageTemplate::addSlotType(SlotType type) { m_types.push_back(type); }
//=============================================================================
cerberus::SlotType MessageTemplate::getSlotTypeAt(size_t index) const { return m_types[index]; }
//=============================================================================
size_t MessageTemplate::count() const { return m_types.size(); }
//=============================================================================
