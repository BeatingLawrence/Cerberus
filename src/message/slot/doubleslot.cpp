#include "doubleslot.h"

using namespace cerberus::message::slot;

//=============================================================================
cerberus::cerberus_slot DoubleSlot::create(double value) { return cerberus_slot(new DoubleSlot(value)); }
//=============================================================================
cerberus::cerberus_slot DoubleSlot::create(const DoubleSlot& other) { return cerberus_slot(new DoubleSlot(other)); }
//=============================================================================
DoubleSlot::DoubleSlot(double value)
    : BaseSlot(ST_DOUBLE),
      m_value(value)
{
    // noop
}
//=============================================================================
double DoubleSlot::value() const { return m_value; }
//=============================================================================
void DoubleSlot::value(double value) { m_value = value; }
//=============================================================================
