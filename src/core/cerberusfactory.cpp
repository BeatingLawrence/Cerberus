#include "cerberusfactory.h"

#include "../message/messagetemplate.h"
#include "../message/slot/charslot.h"
#include "../message/slot/stringslot.h"
#include "src/core/cerberuslog.h"
#include "src/core/cerberusobject.h"
#include "src/core/cerberusregister.h"

using namespace cerberus::core;

//=============================================================================
cerberus::message::slot::cerberus_slot CerberusFactory::slotConstruct(SlotType type)
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

        case ST_BYTEBUFFER:
            // TODO to implement
            break;
    }

    throw cerberusImplMissExc("Missing creation for the given slot type");
}
//=============================================================================
uint32_t CerberusFactory::registerMessage(const message::Message& message, const std::string& name)
{
    auto tmplt = cerberus::message::MessageTemplate(message, name);
    tmplt.registerThis();
    return tmplt.id();
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
        clogError("The requested ID is in the reserved range");
        return message::Message::create();
    }

    auto tmplt = cerberus::core::CerberusRegister::msgTemplateById(id);

    if (!tmplt.isObjValid())
    {
        return message::Message::create();
    }

    message::cerberus_message message = message::Message::create(id);

    for (size_t i = 0; i < tmplt.count(); i++)
    {
        message->addSlot(slotConstruct(tmplt.getSlotTypeAt(i)));
    }

    return message;
}
//=============================================================================
cerberus::message::cerberus_message CerberusFactory::messageConstruct(const std::string& name)
{
    auto tmplt = cerberus::core::CerberusRegister::msgTemplateByName(name);

    if (!tmplt.isObjValid())
    {
        return message::Message::create();
    }

    message::cerberus_message message = message::Message::create(tmplt.id());

    for (size_t i = 0; i < tmplt.count(); i++)
    {
        message->addSlot(slotConstruct(tmplt.getSlotTypeAt(i)));
    }

    return message;
}
//=============================================================================
cerberus::message::cerberus_message CerberusFactory::standardMessageConstruct(StandardMessage type)
{
    message::cerberus_message msg;

    switch (type)
    {
        case SM_LogMsg:
            msg = message::Message::create(CERBERUS_MESSAGE_LOG_ID);
            msg->addSlot(message::slot::StringSlot::create());
            break;

        case SM_TerminationMsg:
            msg = message::Message::create(CERBERUS_MESSAGE_TERM_ID);
            break;

            // add here more message specializations..

        default:
            clogError("Given type does not exist");
            msg = message::Message::create(CERBERUS_INVALID_ID);
            break;
    }

    return msg;
}
//=============================================================================
