#include "messagetemplate.h"

#include "message.h"

using namespace cerberus;

//=============================================================================
MessageTemplate::MessageTemplate(HASH32 id)
    : id(id)
{
}
//=============================================================================
MessageTemplate::MessageTemplate(const MessageTemplate& other)
    : m_name(other.m_name),
      id(other.id)
{
    for (auto& el : other.m_types) m_types.push_back(el.duplicate());
}
//=============================================================================
MessageTemplate::MessageTemplate(const std::string& name)
    : m_name(name),
      id(CERBERUS_INVALID_ID)
{
}
//=============================================================================
MessageTemplate::MessageTemplate(const Message& message, const std::string& name)
    : m_name(name)
{
    for (size_t i = 0; i < message.count(); i++) m_types.push_back(message.getSlotAt(i));
}
//=============================================================================
slot_ptr& MessageTemplate::getSlotAt(size_t index) { return m_types[index]; }
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
msg_ptr MessageTemplate::instantiate() const
{
    auto m = Message::create(id);
    for (const auto& slot : m_types)
    {
        auto s = slot->newslot();
        s->setId(slot->id());
        m->addSlot(std::move(s));
    }
    return std::move(m);
}
//=============================================================================
