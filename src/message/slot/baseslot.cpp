#include "baseslot.h"

using namespace cerberus::message::slot;

//=============================================================================
BaseSlot::BaseSlot(uint32_t id) : m_id(id)
{
    // noop
}
//=============================================================================
BaseSlot::BaseSlot(const BaseSlot& other) : m_id(other.m_id)
{
    // noop
}
//=============================================================================
uint32_t BaseSlot::id() const
{
    return m_id;
}
//=============================================================================
