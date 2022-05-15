#include "messagetemplate.h"
#include "./message.h"
using namespace cerberus::message;

//=============================================================================
MessageTemplate::MessageTemplate(const std::string& name) : m_name(name)
{
    // noop
}
//=============================================================================
MessageTemplate::MessageTemplate(const Message& message, const std::string& name) : m_name(name)
{
    for(size_t i = 0; i < message.count(); i++)
    {
        m_types.push_back(message.getSlotAt(i)->type());
    }
}
//=============================================================================
void MessageTemplate::addSlotType(slot::BaseSlot::SlotType type)
{
    m_types.push_back(type);
}
//=============================================================================
slot::BaseSlot::SlotType MessageTemplate::getSlotTypeAt(size_t index) const
{
    return m_types[index];
}
//=============================================================================
std::string MessageTemplate::name() const
{
    return m_name;
}
//=============================================================================
size_t MessageTemplate::count() const
{
    return m_types.size();
}
//=============================================================================
