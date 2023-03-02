#include "cerberusfactory.h"
#include "../cerberusobject.h"
#include "../message/slot/charslot.h"
#include "../message/slot/stringslot.h"
#include "../message/messagetemplate.h"
#include "src/core/cerberuslog.h"

using namespace cerberus::core;

//=============================================================================
CerberusFactory::CerberusFactory()
{
    // noop
}
//=============================================================================
CerberusFactory::~CerberusFactory()
{
    // noop
}
//=============================================================================
CerberusFactory* CerberusFactory::_instance()
{
    static CerberusFactory factory;
    return &factory;
}
//=============================================================================
cerberus::message::slot::cerberus_slot CerberusFactory::slotFactory(SlotType type)
{
    switch(type)
    {
        case ST_UCHAR:
            // TODO to implement
            break;

        case ST_CHAR:
            return message::slot::CharSlot::create();
            break;

        case ST_USHORT:
            // TODO to implement
            break;

        case ST_SHORT:
            // TODO to implement
            break;

        case ST_ULONG:
            // TODO to implement
            break;

        case ST_LONG:
            // TODO to implement
            break;

        case ST_ULONGLONG:
            // TODO to implement
            break;

        case ST_LONGLONG:
            // TODO to implement
            break;

        case ST_FLOAT:
            // TODO to implement
            break;

        case ST_DOUBLE:
            // TODO to implement
            break;

        case ST_BOOL:
            // TODO to implement
            break;

        case ST_VOIDP:
            // TODO to implement
            break;

        case ST_STDSTRINGP:
            return message::slot::StringSlot::create();
            break;
    }

    throw cerberusIllegalArgumentExc("SlotFactory: Given slot type does not exist");
}
//=============================================================================
uint32_t CerberusFactory::registerMessage(const message::Message& message, const std::string& name)
{
    return (new cerberus::message::MessageTemplate(message, name))->id();
}
//=============================================================================
uint32_t CerberusFactory::messageIdByName(const std::string& name)
{
    CerberusObject* found = _instance()->m_register.cerberusObjectByName(name);

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
cerberus::message::cerberus_message CerberusFactory::messageConstruct(uint32_t id)
{
    if(id == CERBERUS_INVALID_ID)
    {
        throw cerberusIllegalArgumentExc("ID is not valid");
    }

    if(id < CERBERUS_FACTORY_START_ID)
    {
        //reserved range
        debug("Attempt to construct a default message with the factory. Use the StandardMessageFactory instead");
    }

    CerberusObject* found = _instance()->m_register.cerberusObjectByID(id);

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
uint32_t CerberusFactory::threadIdByName(const std::string& name)
{
    CerberusObject* found = _instance()->m_register.cerberusObjectByName(name);

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
cerberus::message::cerberus_message CerberusFactory::createStandardMessage(StandardMessage type)
{
    message::cerberus_message msg;

    switch(type)
    {
        case cerberus::core::CerberusFactory::SM_LogMessage:
            msg = message::Message::create(CERBERUS_MESSAGE_LOG_ID);
            msg->addSlot(message::slot::StringSlot::create());
            break;

        case cerberus::core::CerberusFactory::SM_ShutdownMessage:
            msg = message::Message::create(CERBERUS_MESSAGE_SHUTDOWN_ID);
            break;

        //add here more message specializations..

        default:
            debug("Default message factory given type was not implemented");
            msg = message::Message::create(CERBERUS_INVALID_ID);
            break;
    }

    return msg;
}
//=============================================================================
cerberus::CerberusObject* CerberusFactory::_cerberusObjectById(uint32_t id)
{
    return _instance()->m_register.cerberusObjectByID(id);
}
//=============================================================================
void CerberusFactory::_freeMemory()
{
    _instance()->m_register.freeMemory();
}
//=============================================================================
uint32_t CerberusFactory::_registerCerberusObject(CerberusObject* object)
{
    return _instance()->m_register.registerCerberusObject(object);
}
//=============================================================================
void CerberusFactory::_unregisterCerberusObject(uint32_t id)
{
    _instance()->m_register.unregisterCerberusObject(id);
}
//=============================================================================
