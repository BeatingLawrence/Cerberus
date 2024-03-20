#include "messagetemplate.h"

#include "./message.h"
#include "slot/slot.h"

using namespace cerberus::message;

//=============================================================================
MessageTemplate::MessageTemplate()
    : CerberusObject(CerberusObject::COBJ_Invalid)
{
}
//=============================================================================
MessageTemplate::MessageTemplate(const std::string& name)
    : CerberusObject(CerberusObject::COBJ_MessageTmplt, name)
{
}
//=============================================================================
MessageTemplate::MessageTemplate(const Message& message, const std::string& name)
    : CerberusObject(CerberusObject::COBJ_MessageTmplt, name)
{
    for (size_t i = 0; i < message.count(); i++)
    {
        m_types.push_back({message.getConstSlotAt(i)->type(), message.getConstSlotAt(i)->name()});
    }
}
//=============================================================================
MessageTemplate::~MessageTemplate()
{
    // noop
}
//=============================================================================
MessageTemplate& MessageTemplate::addSlotType(SlotType type, const std::string& name)
{
    m_types.push_back({type, name});
    return *this;
}
//=============================================================================
cerberus::SlotTemplate MessageTemplate::getSlotAt(size_t index) const { return m_types[index]; }
//=============================================================================
size_t MessageTemplate::count() const { return m_types.size(); }
//=============================================================================
