#include "stringslot.h"
#include <string>

using namespace cerberus::message::slot;

//=============================================================================
cerberus_slot StringSlot::create(const std::string& string)
{
    return cerberus_slot(new StringSlot(string));
}
//=============================================================================
cerberus_slot StringSlot::createFrom(const StringSlot& other)
{
    return cerberus_slot(new StringSlot(other));
}
//=============================================================================
StringSlot::StringSlot(const std::string& string) :
    BaseSlot(ST_STDSTRINGP),
    m_string(new std::string(string))
{
    // noop
}
//=============================================================================
StringSlot::StringSlot(const StringSlot& other) :
    BaseSlot(other.type()),
    m_string(new std::string(*other.m_string))
{
    // noop
}
//=============================================================================
StringSlot::~StringSlot()
{
    if(m_string != nullptr)
    {
        delete m_string;
        m_string = nullptr;
    }
}
//=============================================================================
std::string StringSlot::value() const
{
    return std::string(*m_string);
}
//=============================================================================
void StringSlot::setValue(const std::string& value)
{
    m_string->assign(value);
}
//=============================================================================
