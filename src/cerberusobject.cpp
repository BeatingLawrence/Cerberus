#include "cerberusobject.h"
#include "exception/exceptioncatalog.h"
#include "./cerberus.h"

using namespace cerberus;

//=============================================================================
CerberusObject::CerberusObject(uint32_t type, const std::string& name) :
    m_id(CERBERUS_INVALID_ID), m_type(type), m_name(name)
{
    m_id = Cerberus::provider()->_registerCerberusObject(this);
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
template<class T> T* CerberusObject::to()
{
    T* casted = dynamic_cast<T>(this);

    if(casted == nullptr)
    {
        throw cerberusIllegalArgumentExc(cerberus::Cerberus::strPrint("Unable co cast to %s", typeid(T).name()).c_str());
    }

    return casted;
}
//=============================================================================
