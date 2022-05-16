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
Message::Message(uint32_t typeID) : m_slots(), m_typeID(typeID), m_destinationID(CERBERUS_INVALID_ID)
{
    // noop
}
//=============================================================================
Message::Message(const Message& other) : m_slots(other.m_slots), m_typeID(other.m_typeID), m_destinationID(other.m_destinationID)
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
uint32_t Message::typeID() const
{
    return m_typeID;
}
//=============================================================================
uint32_t Message::destinationID() const
{
    return m_destinationID;
}
//=============================================================================
void Message::setDestinationID(uint32_t id)
{
    m_destinationID = id;
}
//=============================================================================
bool Message::isValid() const
{
    return (m_typeID != CERBERUS_INVALID_ID);
}
//=============================================================================
