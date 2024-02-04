#include "baseslot.h"

using namespace cerberus::message::slot;

//=============================================================================
BaseSlot::BaseSlot(SlotType type, const std::string& name)
    : m_type(type),
      m_name(name)
{
    // noop
}
//=============================================================================
BaseSlot::~BaseSlot() {}
//=============================================================================
cerberus::SlotType BaseSlot::type() const { return m_type; }
//=============================================================================
std::string BaseSlot::name() const { return m_name; }
//=============================================================================
