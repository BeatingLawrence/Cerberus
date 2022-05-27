#include "cerberusobject.h"
#include "./define.h"
#include "./core/cerberusfactory.h"

using namespace cerberus;

//=============================================================================
CerberusObject::CerberusObject(ObjectType type, const std::string& name) :
    m_id(CERBERUS_INVALID_ID), m_type(type), m_name(name)
{
    m_id = core::CerberusFactory::_registerCerberusObject(this);
}
//=============================================================================
CerberusObject::~CerberusObject()
{
    core::CerberusFactory::_unregisterCerberusObject(m_id);
}
//=============================================================================
uint32_t CerberusObject::id() const
{
    return m_id;
}
//=============================================================================
CerberusObject::ObjectType CerberusObject::type() const
{
    return m_type;
}
//=============================================================================
std::string CerberusObject::name() const
{
    return m_name;
}
//=============================================================================
