#include "baseslot.h"
#include "../../exception/exceptioncatalog.h"
#include "../../cerberus.h"

using namespace cerberus::message::slot;

//=============================================================================
BaseSlot::BaseSlot(SlotType type) : m_dataType(type), m_id(0)
{
    // noop
}
//=============================================================================
BaseSlot::~BaseSlot()
{
    // noop
}
//=============================================================================
BaseSlot::BaseSlot(const BaseSlot& other) : m_dataType(other.m_dataType), m_id(other.m_id)
{
    // noop
}
//=============================================================================
cerberus::SlotType BaseSlot::type() const
{
    return m_dataType;
}
//=============================================================================
uint32_t BaseSlot::id() const
{
    return m_id;
}
//=============================================================================
void BaseSlot::setId(uint32_t id)
{
    m_id = id;
}
//=============================================================================
