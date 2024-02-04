#include "int32slot.h"

using namespace cerberus::message::slot;

//=============================================================================
cerberus::cerberus_slot Int32Slot::create(int32_t value) { return cerberus_slot(new Int32Slot(value)); }
//=============================================================================
cerberus::cerberus_slot Int32Slot::create(const Int32Slot& other) { return cerberus_slot(new Int32Slot(other)); }
//=============================================================================
Int32Slot::Int32Slot(int32_t value)
    : BaseSlot(ST_INT32),
      m_value(value)
{
    // noop
}
//=============================================================================
int32_t Int32Slot::value() const { return m_value; }
//=============================================================================
void Int32Slot::value(int32_t value) { m_value = value; }
//=============================================================================
