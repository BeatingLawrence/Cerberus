#include "message.h"

#include "../core/cerberusregister.h"  // IWYU pragma: export
#include "../data/bytebuffer.h"
#include "../exception/exceptioncatalog.h"
#include "slot.h"

using namespace cerberus;

//=============================================================================
slot_ptr* Message::_slot(const std::string& name) const
{
    HASH32 h = hashFunc(name);
    for (auto& el : m_slots)
        if (el->id() == h) return &el;

    return nullptr;
}
//=============================================================================
msg_ptr Message::create(HASH32 id) { return msg_ptr(new Message(id)); }
//=============================================================================
msg_ptr Message::create(const std::string& name) { return msg_ptr(new Message(name)); }
//=============================================================================
Message::Message(HASH32 id)
    : m_id(id),
      m_recipientId(CERBERUS_INVALID_ID)
{
    // noop
}
//=============================================================================
Message::Message(const std::string& name)
    : m_id(hashFunc_res(name)),
      m_recipientId(CERBERUS_INVALID_ID)
{
    // noop
}
//=============================================================================
Message::Message(const Message& other)
    : m_id(other.m_id),
      m_recipientId(other.m_recipientId)
{
    // deep-copy all slots
    for (auto& el : other.m_slots) m_slots.push_back(el.duplicate());
}
//=============================================================================
Message::~Message() {}
//=============================================================================
size_t Message::count() const { return m_slots.size(); }
//=============================================================================
Message& Message::addSlot(const slot_ptr& slot)
{
    if (!slot) throw cIllegalArgExc("slot is null");
    m_slots.push_back(std::move(slot.duplicate()));
    return *this;
}
//=============================================================================
Message& Message::addSlot(slot_ptr&& slot)
{
    if (!slot) throw cIllegalArgExc("slot is null");
    m_slots.push_back(std::move(slot));
    return *this;
}
//=============================================================================
slot_ptr& Message::getSlotAt(size_t index)
{
    if (index >= m_slots.size()) throw cIllegalArgExc("Index out of boundaries");
    return m_slots[index];
}
//=============================================================================
slot_ptr Message::getSlotAt(size_t index) const
{
    if (index >= m_slots.size()) throw cIllegalArgExc("Index out of boundaries");
    return m_slots[index].duplicate();
}
//=============================================================================
slot_ptr& Message::getSlot(const std::string& name)
{
    auto p = _slot(name);
    if (!p) throw cIllegalArgExc("Slot \"%s\" not found", name.c_str());

    return *p;
}
//=============================================================================
slot_ptr Message::getSlot(const std::string& name) const
{
    auto p = _slot(name);
    if (!p) throw cIllegalArgExc("Slot \"%s\" not found", name.c_str());

    return p->duplicate();
}
//=============================================================================
HASH32 Message::id() const { return m_id; }
//=============================================================================
HASH32 Message::idFromName(const std::string& name) { return hashFunc_res(name); }
//=============================================================================
bool Message::is(const std::string& name) const
{
    if (name.empty()) return false;

    return m_id == hashFunc_res(name);
}
//=============================================================================
HASH32 Message::recipient() const { return m_recipientId; }
//=============================================================================
bool Message::hasValidRecipient() const { return m_recipientId != CERBERUS_INVALID_ID; }
//=============================================================================
Message& Message::setRecipient(HASH32 id)
{
    m_recipientId = id;
    return *this;
}
//=============================================================================
ByteBuffer Message::toBuffer() const
{
    ByteBuffer ret;
    for (auto& el : m_slots) ret.append(el->toBuffer());
    return ret;
}
//=============================================================================
ConstIterator<slot_ptr> Message::begin() const
{
    if (m_slots.empty()) return nullptr;
    return &m_slots.front();
}
//=============================================================================
ConstIterator<slot_ptr> Message::end() const
{
    if (m_slots.empty()) return nullptr;
    return ((&m_slots.back()) + 1);
}
//=============================================================================
Clonable* Message::clone() const { return new Message(*this); }
//=============================================================================
SIZE Message::memfp() const
{
    SIZE s = sizeof(Message);
    for (auto&& el : m_slots) s += el.memFootprint();
    return s;
}
//=============================================================================
