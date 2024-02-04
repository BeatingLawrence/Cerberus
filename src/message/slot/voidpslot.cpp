#include "voidpslot.h"

using namespace cerberus::message::slot;

//=============================================================================
cerberus::cerberus_slot VoidPSlot::create(void* value) { return cerberus_slot(new VoidPSlot(value)); }
//=============================================================================
cerberus::cerberus_slot VoidPSlot::create(const VoidPSlot& other) { return cerberus_slot(new VoidPSlot(other)); }
//=============================================================================
VoidPSlot::VoidPSlot(void* value)
    : BaseSlot(ST_VOIDP),
      m_value(value)
{
    // noop
}
//=============================================================================
void* VoidPSlot::value() const { return m_value; }
//=============================================================================
void VoidPSlot::value(void* value) { m_value = value; }
//=============================================================================
