#include "bytebufferslot.h"

using namespace cerberus::message::slot;

//=============================================================================
cerberus::cerberus_slot ByteBufferSlot::create(const data::ByteBuffer& value) { return cerberus_slot(new ByteBufferSlot(value)); }
//=============================================================================
cerberus::cerberus_slot ByteBufferSlot::create(const ByteBufferSlot& other) { return cerberus_slot(new ByteBufferSlot(other)); }
//=============================================================================
ByteBufferSlot::ByteBufferSlot(const data::ByteBuffer& value)
    : BaseSlot(ST_BYTEBUFFER),
      m_value(value)
{
    // noop
}
//=============================================================================
const cerberus::data::ByteBuffer& ByteBufferSlot::value() const { return m_value; }
//=============================================================================
cerberus::data::ByteBuffer& ByteBufferSlot::value() { return m_value; }
//=============================================================================
void ByteBufferSlot::value(const data::ByteBuffer& value) { m_value.assign(value); }
//=============================================================================
