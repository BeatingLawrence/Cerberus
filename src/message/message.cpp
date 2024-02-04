#include "message.h"

#include "../exception/exceptioncatalog.h"
#include "./slot/baseslot.h"

using namespace cerberus::message;

//=============================================================================
cerberus::cerberus_message Message::create(uint32_t typeID) { return cerberus_message(new Message(typeID)); }
//=============================================================================
cerberus::cerberus_message Message::createFrom(const Message& other) { return cerberus_message(new Message(other)); }
//=============================================================================
Message::Message(uint32_t typeID)
    : m_slots(),
      m_id(typeID),
      m_destinationId(CERBERUS_INVALID_ID)
{
    // noop
}
//=============================================================================
size_t Message::count() const { return m_slots.size(); }
//=============================================================================
void Message::addSlot(cerberus_slot slot) { m_slots.push_back(slot); }
//=============================================================================
cerberus::cerberus_slot Message::getSlotAt(size_t index) const
{
    if (index >= m_slots.size())
    {
        throw cerberusIllegalArgExc("Index out of boundaries");
    }

    return m_slots[index];
}
//=============================================================================
cerberus::cerberus_slot Message::getSlot(const std::string& name) const
{
    for (auto& el : m_slots)
    {
        if (core::CerberusUtils::areEqual(el->name(), name))
        {
            return el;
        }
    }

    return cerberus_slot();
}
//=============================================================================
uint32_t Message::id() const { return m_id; }
//=============================================================================
uint32_t Message::destination() const { return m_destinationId; }
//=============================================================================
void Message::setDestination(uint32_t id) { m_destinationId = id; }
//=============================================================================
bool Message::isValid() const { return (m_id != CERBERUS_INVALID_ID); }
//=============================================================================
