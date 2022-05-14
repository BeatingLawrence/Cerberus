#include "message.h"
#include "../exception/exceptioncatalog.h"
#include "../cerberus.h"

using namespace cerberus::message;

//=============================================================================
cerberus_message Message::create()
{
    return cerberus_message(new Message());
}
//=============================================================================
cerberus_message Message::createFrom(const Message& other)
{
    return cerberus_message(new Message(other));
}
//=============================================================================
Message::Message() : m_slots(), m_id(cerberus::Cerberus::Invalid_ID)
{
    // noop
}
//=============================================================================
Message::Message(const Message& other) : m_slots(other.m_slots), m_id(other.m_id)
{
    // noop
}
//=============================================================================
size_t Message::count() const
{
    return m_slots.size();
}
//=============================================================================
void Message::addSlot(slot::cerberus_slot slot)
{
    m_slots.push_back(slot);
}
//=============================================================================
slot::cerberus_slot Message::getSlotAt(size_t index) const
{
    if(index >= m_slots.size())
    {
        throw cerberusIllegalArgumentExc("Index out of boundaries");
    }

    return m_slots[index];
}
//=============================================================================
slot::cerberus_slot Message::getSlotById(uint32_t id) const
{
    for(auto& el : m_slots)
    {
        if(el->id() == id)
        {
            return el;
        }
    }

    return slot::cerberus_slot();
}
//=============================================================================
void Message::setId(uint32_t id)
{
    m_id = id;
}
//=============================================================================
uint32_t Message::id() const
{
    return m_id;
}
//=============================================================================
bool Message::isValid() const
{
    return (m_id != cerberus::Cerberus::Invalid_ID);
}
//=============================================================================
