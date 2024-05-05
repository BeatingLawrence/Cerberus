#include "messagetemplate.h"

#include "message.h"
#include "slot.h"

using namespace cerberus;

//=============================================================================
MessageTemplate::MessageTemplate() {}
//=============================================================================
MessageTemplate::MessageTemplate(const std::string& name)
    : m_name(name)
{
}
//=============================================================================
MessageTemplate::MessageTemplate(const Message& message, const std::string& name)
    : m_name(name)
{
    for (size_t i = 0; i < message.count(); i++)
    {
        m_types.push_back({message.getConstSlotAt(i)->type(), message.getConstSlotAt(i)->name()});
    }
}
//=============================================================================
MessageTemplate& MessageTemplate::addSlotType(SlotType type, const std::string& name)
{
    m_types.push_back({type, name});
    return *this;
}
//=============================================================================
MessageTemplate::SlotTemplate MessageTemplate::getSlotAt(size_t index) const { return m_types[index]; }
//=============================================================================
size_t MessageTemplate::count() const { return m_types.size(); }
//=============================================================================
std::string MessageTemplate::name() const { return m_name; }
//=============================================================================
