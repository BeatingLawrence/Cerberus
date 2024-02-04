#include "byteslot.h"

using namespace cerberus::message::slot;

//=============================================================================
cerberus::cerberus_slot ByteSlot::create(BYTE value) { return cerberus_slot(new ByteSlot(value)); }
//=============================================================================
cerberus::cerberus_slot ByteSlot::create(const ByteSlot& other) { return cerberus_slot(new ByteSlot(other)); }
//=============================================================================
ByteSlot::ByteSlot(BYTE value)
    : BaseSlot(ST_BYTE),
      m_value(value)
{
    // noop
}
//=============================================================================
cerberus::BYTE ByteSlot::value() const { return m_value; }
//=============================================================================
void ByteSlot::value(BYTE value) { m_value = value; }
//=============================================================================
