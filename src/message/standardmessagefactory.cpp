#include "./standardmessagefactory.h"
#include "../exception/exceptioncatalog.h"
#include "./slot/stringslot.h"

//=============================================================================
cerberus::message::cerberus_message cerberus::message::StandardMessageFactory::createStandardMessage(uint32_t id)
{
    message::cerberus_message msg = message::Message::create(id);

    switch(id)
    {
        case CERBERUS_MESSAGE_SHUTDOWN_ID:
        {
            return msg;
        }
        break;

        case CERBERUS_MESSAGE_LOG_ID:
        {
            msg->addSlot(slot::StringSlot::create());
            return msg;
        }
        break;
            //add here more message specializations..
    }

    throw cerberusIllegalArgumentExc("Default-message factory given ID does not exist");
}
//=============================================================================
