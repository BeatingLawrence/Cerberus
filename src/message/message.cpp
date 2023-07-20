#include "message.h"
#include "../exception/exceptioncatalog.h"
#include "./slot/baseslot.h"

using namespace cerberus::message;

//=============================================================================
cerberus_message Message::create(uint32_t typeID)
{
    return cerberus_message(new Message(typeID));
}
//=============================================================================
cerberus_message Message::createFrom(const Message& other)
{
    return cerberus_message(new Message(other));
}
//=============================================================================
Message::Message(uint32_t typeID) : m_slots(), m_id(typeID), m_destinationId(CERBERUS_INVALID_ID)
{
    // noop
}
//=============================================================================
Message::Message(const Message& other) : m_slots(other.m_slots), m_id(other.m_id), m_destinationId(other.m_destinationId)
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
        throw cerberusIllegalArgExc("Index out of boundaries");
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
uint32_t Message::id() const
{
    return m_id;
}
//=============================================================================
uint32_t Message::destinationId() const
{
    return m_destinationId;
}
//=============================================================================
void Message::setDestinationId(uint32_t id)
{
    m_destinationId = id;
}
//=============================================================================
bool Message::isValid() const
{
    return (m_id != CERBERUS_INVALID_ID);
}
//=============================================================================
