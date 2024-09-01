#include "slot.h"

using namespace cerberus;

//=============================================================================
BaseSlot::BaseSlot(const std::string& name)
    : m_name(name)
{
}
//=============================================================================
BaseSlot::~BaseSlot() {}
//=============================================================================
std::string BaseSlot::name() const { return m_name; }
//=============================================================================
BaseSlot& BaseSlot::name(const std::string& name)
{
    m_name = name;
    return *this;
}
//=============================================================================
slot_ptr BaseSlot::newslot(const std::string name)
{
    auto p = newslot();
    p->name(name);
    return p;
}
//=============================================================================
