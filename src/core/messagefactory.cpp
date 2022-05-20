#include "messagefactory.h"
#include "../cerberusobject.h"
#include "../message/slot/charslot.h"
#include "../message/slot/stringslot.h"
#include "../message/messagetemplate.h"
#include "../message/standardmessagefactory.h"

using namespace cerberus::core;

//=============================================================================
MessageFactory::MessageFactory()
{
}
//=============================================================================
cerberus::message::slot::cerberus_slot MessageFactory::slotFactory(message::slot::SlotType type)
{
    switch(type)
    {
        case message::slot::ST_UCHAR:
            // TODO to implement
            break;

        case message::slot::ST_CHAR:
            return message::slot::CharSlot::create();
            break;

        case message::slot::ST_USHORT:
            // TODO to implement
            break;

        case message::slot::ST_SHORT:
            // TODO to implement
            break;

        case message::slot::ST_ULONG:
            // TODO to implement
            break;

        case message::slot::ST_LONG:
            // TODO to implement
            break;

        case message::slot::ST_ULONGLONG:
            // TODO to implement
            break;

        case message::slot::ST_LONGLONG:
            // TODO to implement
            break;

        case message::slot::ST_FLOAT:
            // TODO to implement
            break;

        case message::slot::ST_DOUBLE:
            // TODO to implement
            break;

        case message::slot::ST_BOOL:
            // TODO to implement
            break;

        case message::slot::ST_VOIDP:
            // TODO to implement
            break;

        case message::slot::ST_STDSTRINGP:
            return message::slot::StringSlot::create();
            break;
    }

    throw cerberusIllegalArgumentExc("SlotFactory: Given slot type does not exist");
}
//=============================================================================
uint32_t MessageFactory::registerMessage(const message::Message& message, const std::string& name)
{
    return (new cerberus::message::MessageTemplate(message, name))->id();
}
//=============================================================================
uint32_t MessageFactory::messageIdByName(const std::string& name)
{
    CerberusObject* found = m_register.cerberusObjectByName(name);

    if(found == nullptr)
    {
        return CERBERUS_INVALID_ID;
    }
    else
    {
        if(found->type() == CerberusObject::ObjectType::OT_MessageTemplate)
        {
            return found->id();
        }
        else
        {
            return CERBERUS_INVALID_ID;
        }
    }
}
//=============================================================================
cerberus::message::cerberus_message MessageFactory::messageConstruct(uint32_t id)
{
    if(id == CERBERUS_INVALID_ID)
    {
        throw cerberusIllegalArgumentExc("ID is not valid");
    }

    if(id < CERBERUS_FACTORY_START_ID)
    {
        //reserved range
        return message::StandardMessageFactory::createStandardMessage(id);
    }

    CerberusObject* found = m_register.cerberusObjectByID(id);

    if(found == nullptr)
    {
        throw cerberusIllegalArgumentExc("Factory given ID does not exist");
    }

    if(found->type() != CerberusObject::ObjectType::OT_MessageTemplate)
    {
        throw cerberusIllegalArgumentExc("Factory given ID is not a message ID");
    }

    message::MessageTemplate* tmplt = found->to<message::MessageTemplate>();
    message::cerberus_message message = message::Message::create(found->id());

    for(size_t i = 0; i < tmplt->count(); i++)
    {
        message->addSlot(slotFactory(tmplt->getSlotTypeAt(i)));
    }

    return message;
}
//=============================================================================
uint32_t MessageFactory::threadIdByName(const std::string& name)
{
    CerberusObject* found = m_register.cerberusObjectByName(name);

    if(found == nullptr)
    {
        return CERBERUS_INVALID_ID;
    }
    else
    {
        if(found->type() == CerberusObject::ObjectType::OT_Thread)
        {
            return found->id();
        }
        else
        {
            return CERBERUS_INVALID_ID;
        }
    }
}
//=============================================================================
cerberus::CerberusObject* MessageFactory::cerberusObjectById(uint32_t id) const
{
    return m_register.cerberusObjectByID(id);
}
//=============================================================================
void MessageFactory::freeMemory()
{
    m_register.freeMemory();
}
//=============================================================================
uint32_t MessageFactory::registerCerberusObject(CerberusObject* object)
{
    return m_register.registerCerberusObject(object);
}
//=============================================================================
void MessageFactory::unregisterCerberusObject(uint32_t id)
{
    m_register.unregisterCerberusObject(id);
}
//=============================================================================
