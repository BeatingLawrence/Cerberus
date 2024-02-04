#include "int64slot.h"

using namespace cerberus::message::slot;

//=============================================================================
cerberus::cerberus_slot Int64Slot::create(int64_t value) { return cerberus_slot(new Int64Slot(value)); }
//=============================================================================
cerberus::cerberus_slot Int64Slot::create(const Int64Slot& other) { return cerberus_slot(new Int64Slot(other)); }
//=============================================================================
Int64Slot::Int64Slot(int64_t value)
    : BaseSlot(ST_INT64),
      m_value(value)
{
    // noop
}
//=============================================================================
int64_t Int64Slot::value() const { return m_value; }
//=============================================================================
void Int64Slot::value(int64_t value) { m_value = value; }
//=============================================================================
