#include "message.h"

#include "../core/cerberusregister.h"  // IWYU pragma: export
#include "../cerberus.h"
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
Message::Message(HASH32 id)
    : m_id(id),
      m_recipientIds()
{
    // noop
}
//=============================================================================
Message::Message(const Message& other)
    : m_id(other.m_id),
      m_recipientIds(other.m_recipientIds)
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
bool Message::is(HASH32 id) const { return m_id == id; }
//=============================================================================
HASH32 Message::recipient() const
{
    if (m_recipientIds.empty()) return CERBERUS_INVALID_ID;
    return m_recipientIds.front();
}
//=============================================================================
const std::vector<HASH32>& Message::recipients() const { return m_recipientIds; }
//=============================================================================
bool Message::hasValidRecipient() const
{
    for (const auto& id : m_recipientIds)
        if (id != CERBERUS_INVALID_ID) return true;

    return false;
}
//=============================================================================
void Message::setRecipient(HASH32 id) const
{
    m_recipientIds.clear();
    m_recipientIds.push_back(id);
}
//=============================================================================
void Message::setRecipients(const std::vector<HASH32>& ids) const { m_recipientIds = ids; }
//=============================================================================
void Message::addRecipient(HASH32 id) const { m_recipientIds.push_back(id); }
//=============================================================================
void Message::clearRecipients() const { m_recipientIds.clear(); }
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
