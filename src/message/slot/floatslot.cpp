#include "floatslot.h"

using namespace cerberus::message::slot;

//=============================================================================
cerberus::cerberus_slot FloatSlot::create(float value) { return cerberus_slot(new FloatSlot(value)); }
//=============================================================================
cerberus::cerberus_slot FloatSlot::create(const FloatSlot& other) { return cerberus_slot(new FloatSlot(other)); }
//=============================================================================
FloatSlot::FloatSlot(float value)
    : BaseSlot(ST_FLOAT),
      m_value(value)
{
    // noop
}
//=============================================================================
float FloatSlot::value() const { return m_value; }
//=============================================================================
void FloatSlot::value(float value) { m_value = value; }
//=============================================================================
