#include "message.h"

#include "../exception/exceptioncatalog.h"
#include "slot.h"
#include "src/data/bytebuffer.h"
#include "src/data/jsondata.h"

using namespace cerberus;

//=============================================================================
cerberus_message Message::create(HASH32 typeID) { return cerberus_message(new Message(typeID)); }
//=============================================================================
Message::Message(HASH32 typeID)
    : m_slots(),
      m_id(typeID),
      m_recipientId(CERBERUS_INVALID_ID)
{
    // noop
}
//=============================================================================
Message::~Message() {}
//=============================================================================
size_t Message::count() const { return m_slots.size(); }
//=============================================================================
Message& Message::addSlot(slot_ptr slot)
{
    m_slots.push_back(slot);
    return *this;
}
//=============================================================================
Message& Message::clear()
{
    m_slots.clear();
    return *this;
}
//=============================================================================
slot_ptr Message::getSlotAt(size_t index)
{
    if (index >= m_slots.size()) throw cIllegalArgExc("Index out of boundaries");

    return m_slots[index];
}
//=============================================================================
slot_ptr Message::getConstSlotAt(size_t index) const
{
    if (index >= m_slots.size()) throw cIllegalArgExc("Index out of boundaries");

    return m_slots[index];
}
//=============================================================================
slot_ptr Message::getSlot(const std::string& name)
{
    for (auto& el : m_slots)
        if (CerberusUtils::areEqual(el->name(), name)) return el;

    throw cIllegalArgExc("Slot %s does not exist in this message", name.c_str());
}
//=============================================================================
slot_ptr Message::getConstSlot(const std::string& name) const
{
    for (auto& el : m_slots)
        if (CerberusUtils::areEqual(el->name(), name)) return el;

    throw cIllegalArgExc("Slot %s does not exist in this message", name.c_str());
}
//=============================================================================
HASH32 Message::id() const { return m_id; }
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
bool Message::isValid() const { return (m_id != CERBERUS_INVALID_ID); }
//=============================================================================
ByteBuffer Message::toBuffer() const
{
    ByteBuffer ret;

    for (auto& el : m_slots)
    {
        ret.append(el->toBuffer());
    }

    return ret;
}
//=============================================================================
Clonable* Message::clone() const { return new Message(*this); }
/*
//=============================================================================
Message& Message::fill(std::initializer_list<TypeWrapper> values)
{
    if (values.size() != m_slots.size()) throw cIllegalArgExc("fill() called with a wrong number of args");

    size_t index = 0;

    for (auto&& el : values)
    {
        switch (el.type)
        {
            case ST_BYTE:
                getSlotAt(index)->to<ByteSlot>()->value(el._byte);
                break;
            case ST_INT32:
                getSlotAt(index)->to<Int32Slot>()->value(el._int32);
                break;
            case ST_INT64:
                getSlotAt(index)->to<Int64Slot>()->value(el._int64);
                break;
            case ST_FLOAT:
                getSlotAt(index)->to<FloatSlot>()->value(el._float);
                break;
            case ST_DOUBLE:
                getSlotAt(index)->to<DoubleSlot>()->value(el._double);
                break;
            case ST_BOOL:
                getSlotAt(index)->to<BoolSlot>()->value(el._bool);
                break;
            case ST_VOIDP:
                getSlotAt(index)->to<VoidPSlot>()->value(el._voidp);
                break;
            case ST_STRING:
                getSlotAt(index)->to<StringSlot>()->value(*((std::string*)el._voidp));
                break;
            case ST_BYTEBUFFER:
                getSlotAt(index)->to<BufferSlot>()->value(*((ByteBuffer*)el._voidp));
                break;
            case ST_DICTIONARY:
                getSlotAt(index)->to<DictionarySlot>()->value(*((Dictionary*)el._voidp));
                break;
            case ST_JSON:
                getSlotAt(index)->to<JsonSlot>()->value(*((JsonData*)el._voidp));
                break;

            default:
                throw cImplMissExc("The given slot type has not been implemented");
        }
    }

    return *this;
}
//=============================================================================
Message& Message::insert(std::initializer_list<TypeWrapper> values)
{
    clear();

    for (auto&& el : values)
    {
        slot_ptr s;

        switch (el.type)
        {
            case ST_BYTE:
                s = ByteSlot::create(el._byte);
                break;
            case ST_INT32:
                s = Int32Slot::create(el._int32);
                break;
            case ST_INT64:
                s = Int64Slot::create(el._int64);
                break;
            case ST_FLOAT:
                s = FloatSlot::create(el._float);
                break;
            case ST_DOUBLE:
                s = DoubleSlot::create(el._double);
                break;
            case ST_BOOL:
                s = BoolSlot::create(el._bool);
                break;
            case ST_VOIDP:
                s = VoidPSlot::create(el._voidp);
                break;
            case ST_STRING:
                s = StringSlot::create(*((std::string*)el._voidp));
                break;
            case ST_BYTEBUFFER:
                s = BufferSlot::create(*((ByteBuffer*)el._voidp));
                break;
            case ST_DICTIONARY:
                s = DictionarySlot::create(*((Dictionary*)el._voidp));
                break;
            case ST_JSON:
                s = JsonSlot::create(*((JsonData*)el._voidp));
                break;

            default:
                throw cImplMissExc("The given slot type has not been implemented");
        }

        m_slots.push_back(s);
    }

    return *this;
}
*/
//=============================================================================
