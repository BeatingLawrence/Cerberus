#include "boolslot.h"

using namespace cerberus::message::slot;

//=============================================================================
cerberus::cerberus_slot BoolSlot::create(bool value) { return cerberus_slot(new BoolSlot(value)); }
//=============================================================================
cerberus::cerberus_slot BoolSlot::create(const BoolSlot& other) { return cerberus_slot(new BoolSlot(other)); }
//=============================================================================
BoolSlot::BoolSlot(bool value)
    : BaseSlot(ST_BOOL),
      m_value(value)
{
    // noop
}
//=============================================================================
bool BoolSlot::value() const { return m_value; }
//=============================================================================
void BoolSlot::value(bool value) { m_value = value; }
//=============================================================================
