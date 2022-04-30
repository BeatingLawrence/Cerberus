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
BaseSlot::BaseSlot(const BaseSlot& other) : m_dataType(other.m_dataType), m_id(other.m_id)
{
    // noop
}
//=============================================================================
BaseSlot::SlotType BaseSlot::type() const
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
template<class T> T* BaseSlot::to()
{
    T* casted = dynamic_cast<T>(this);

    if(casted == nullptr)
    {
        throw cerberusIllegalArgumentExc(cerberus::Cerberus::strPrint("Unable co cast to %s", typeid(T).name()).c_str());
    }

    return casted;
}
//=============================================================================
template<class T> std::shared_ptr<T> BaseSlot::toShared(const cerberus_slot& from)
{
    std::shared_ptr<T> casted = std::dynamic_pointer_cast<T, BaseSlot>(from);

    if(*casted == nullptr)
    {
        throw cerberusIllegalArgumentExc(cerberus::Cerberus::strPrint("Unable co cast to shared_ptr of %s", typeid(T).name()).c_str());
    }

    return casted;
}
//=============================================================================
