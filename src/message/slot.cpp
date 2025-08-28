#include "slot.h"

#include "../core/cerberusutils.h"
#include "../define.h"

using namespace cerberus;

//=============================================================================
SlotBase::SlotBase(const std::string& name)
    : m_id(hashFunc(name))
{
}
//=============================================================================
SlotBase::SlotBase(HASH32 id)
    : m_id(id)
{
}
//=============================================================================
SlotBase::~SlotBase() {}
//=============================================================================
SlotBase& SlotBase::setId(const std::string& name)
{
    m_id = hashFunc(name);
    return *this;
}
//=============================================================================
SlotBase& SlotBase::setId(HASH32 id)
{
    m_id = id;
    return *this;
}
//=============================================================================
HASH32 SlotBase::id() const { return m_id; }
//=============================================================================
