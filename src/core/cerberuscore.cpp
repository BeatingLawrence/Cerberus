#include "cerberuscore.h"

#include "../cerberus.h"
#include "../thread/thread.h"

using namespace cerberus::core;

//=============================================================================
int CerberusCore::tick()
{
    message::cerberus_message message = nextMessage();

    if (!(message->isValid()))
    {
        return 0;
    }

    // Process message queue..
    uint32_t destination = message->destinationId();

    if (destination == CERBERUS_INVALID_ID)
    {
        logDebug("Destination of message is invalid, dropping");
    }
    else
    {
        Cerberus::sendMsgToObj(destination, message);
    }

    // Do other stuff..
    return 0;
}
//=============================================================================
void CerberusCore::warmUp()
{
    logInfo("Starting Core Thread");
    logInfo("Starting Event Scheduler");
    m_eventScheduler.start();
}
//=============================================================================
void CerberusCore::coolDown()
{
    logInfo("Stopping event scheduler");
    m_eventScheduler.join(true);
    //
    logInfo("Stopping Cerberus core");
}
//=============================================================================
CerberusCore::CerberusCore()
    : cerberus::thread::Thread(TP_NonPeriodic, "Cerberus Core")
{
}
//=============================================================================
CerberusCore::~CerberusCore() {}
//=============================================================================
