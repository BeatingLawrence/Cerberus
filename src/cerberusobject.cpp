#include "cerberusobject.h"

using namespace cerberus;

//=============================================================================
CerberusObject::CerberusObject(uint32_t type, const std::string& name) :
    m_id(CERBERUS_INVALID_ID), m_type(type), m_name(name)
{
    m_id = Cerberus::_provider()->_registerCerberusObject(this);
}
//=============================================================================
CerberusObject::~CerberusObject()
{
    Cerberus::_provider()->_unregisterCerberusObject(m_id);
}
//=============================================================================
uint32_t CerberusObject::id() const
{
    return m_id;
}
//=============================================================================
uint32_t CerberusObject::type() const
{
    return m_type;
}
//=============================================================================
std::string CerberusObject::name() const
{
    return m_name;
}
//=============================================================================
