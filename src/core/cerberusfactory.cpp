#include "cerberusfactory.h"

#include "../message/messagetemplate.h"
#include "../message/slot/charslot.h"
#include "../message/slot/stringslot.h"
#include "src/core/cerberuslog.h"
#include "src/core/cerberusobject.h"

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
    switch (type)
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

    throw cerberusIllegalArgExc("SlotFactory: Given slot type does not exist");
}
//=============================================================================
uint32_t CerberusFactory::registerMessage(const message::Message& message, const std::string& name)
{
    return (new cerberus::message::MessageTemplate(message, name))->id();
}
//=============================================================================
uint32_t CerberusFactory::messageIdByName(const std::string& name)
{
    CerberusObject* found = _instance()->m_register.objectByName(name);

    if (found == nullptr)
    {
        return CERBERUS_INVALID_ID;
    }
    else
    {
        if (found->type() == CerberusObject::ObjectType::MessageTemplate)
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
    if (id == CERBERUS_INVALID_ID)
    {
        return message::Message::create();
    }

    if (id < CERBERUS_FACTORY_START_ID)
    {
        // reserved range
        debug("Cannot construct a standard message. Use the standardMessageConstruct() instead");
        return message::Message::create();
    }

    CerberusObject* found = _instance()->m_register.objectById(id);

    if (found == nullptr)
    {
        return message::Message::create();
    }

    if (found->type() != CerberusObject::ObjectType::MessageTemplate)
    {
        return message::Message::create();
    }

    message::MessageTemplate* tmplt = found->to_p<message::MessageTemplate>();
    message::cerberus_message message = message::Message::create(found->id());

    for (size_t i = 0; i < tmplt->count(); i++)
    {
        message->addSlot(slotFactory(tmplt->getSlotTypeAt(i)));
    }

    return message;
}
//=============================================================================
uint32_t CerberusFactory::threadIdByName(const std::string& name)
{
    CerberusObject* found = _instance()->m_register.objectByName(name);

    if (found == nullptr)
    {
        return CERBERUS_INVALID_ID;
    }
    else
    {
        if (found->type() == CerberusObject::ObjectType::Thread)
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
cerberus::message::cerberus_message CerberusFactory::standardMessageConstruct(StandardMessage type)
{
    message::cerberus_message msg;

    switch (type)
    {
        case cerberus::core::CerberusFactory::SM_LogMessage:
            msg = message::Message::create(CERBERUS_MESSAGE_LOG_ID);
            msg->addSlot(message::slot::StringSlot::create());
            break;

        case cerberus::core::CerberusFactory::SM_ShutdownMessage:
            msg = message::Message::create(CERBERUS_MESSAGE_SHUTDOWN_ID);
            break;

            // add here more message specializations..

        default:
            debug("Default message factory given type was not implemented");
            msg = message::Message::create(CERBERUS_INVALID_ID);
            break;
    }

    return msg;
}
//=============================================================================
cerberus::CerberusObject* CerberusFactory::_cerberusObjectById(uint32_t id) { return _instance()->m_register.objectById(id); }
//=============================================================================
uint32_t CerberusFactory::_registerCerberusObject(CerberusObject* object) { return _instance()->m_register.registerObject(object); }
//=============================================================================
void CerberusFactory::_unregisterCerberusObject(uint32_t id) { _instance()->m_register.unregisterObject(id); }
//=============================================================================
