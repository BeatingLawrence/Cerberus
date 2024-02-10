#include "jsonslot.h"

using namespace cerberus::message::slot;

//=============================================================================
cerberus::cerberus_slot JsonSlot::create(const data::JsonData& value) { return cerberus_slot(new JsonSlot(value)); }
//=============================================================================
cerberus::cerberus_slot JsonSlot::create(const JsonSlot& other) { return cerberus_slot(new JsonSlot(other)); }
//=============================================================================
JsonSlot::JsonSlot(const data::JsonData& value)
    : BaseSlot(ST_JSON),
      m_value(value)
{
    // noop
}
//=============================================================================
const cerberus::data::JsonData& JsonSlot::value() const { return m_value; }
//=============================================================================
cerberus::data::JsonData& JsonSlot::value() { return m_value; }
//=============================================================================
void JsonSlot::value(const data::JsonData& value) { m_value = value; }
//=============================================================================
