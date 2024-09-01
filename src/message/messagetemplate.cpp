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
    for (size_t i = 0; i < message.count(); i++) m_types.push_back(message.getConstSlotAt(i));
}
//=============================================================================
MessageTemplate& MessageTemplate::addSlotType(slot_ptr type, const std::string& name)
{
    m_types.push_back(type);
    return *this;
}
//=============================================================================
slot_ptr MessageTemplate::getSlotAt(size_t index) const { return m_types[index]; }
//=============================================================================
size_t MessageTemplate::count() const { return m_types.size(); }
//=============================================================================
std::string MessageTemplate::name() const { return m_name; }
//=============================================================================
ConstIterator<slot_ptr> MessageTemplate::begin() const
{
    if (m_types.empty()) return nullptr;
    return &m_types.front();
}
//=============================================================================
ConstIterator<slot_ptr> MessageTemplate::end() const
{
    if (m_types.empty()) return nullptr;
    return ((&m_types.back()) + 1);
}
//=============================================================================
