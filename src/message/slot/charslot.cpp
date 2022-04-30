#include "charslot.h"
#include "../../define.h"

using namespace cerberus::message::slot;

//=============================================================================
cerberus_slot CharSlot::create(char value)
{
    return cerberus_slot(new CharSlot(value));
}
//=============================================================================
cerberus_slot CharSlot::createFrom(const CharSlot& other)
{
    return cerberus_slot(new CharSlot(other));
}
//=============================================================================
CharSlot::CharSlot(char value) : BaseSlot(BaseSlot::ST_CHAR), m_value(value)
{
    // noop
}
//=============================================================================
CharSlot::CharSlot(const CharSlot& other) : BaseSlot(other.type()), m_value(other.m_value)
{
    // noop
}
//=============================================================================
char CharSlot::value() const
{
    return m_value;
}
//=============================================================================
void CharSlot::setValue(char value)
{
    m_value = value;
}
//=============================================================================
