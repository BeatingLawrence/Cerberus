#include "cerberuscore.h"

#include "../cerberus.h"
#include "../thread/thread.h"

using namespace cerberus::core;

//=============================================================================
void CerberusCore::initializeThreadPool()
{
    if (m_conf.threadPool == 0) return;

    logInfo("Building thread pool");

    m_pool.build(m_conf.threadPool, 10000);
}
//=============================================================================
void CerberusCore::deinitializeThreadPool()
{
    if (m_conf.threadPool == 0) return;

    logInfo("Deallocating thread pool");

    m_pool.clear();
}
//=============================================================================
int CerberusCore::tick()
{
    cerberus_message message = nextMessage();

    if (!(message->isValid())) return 0;

    // Process message queue..

    if (message->hasValidRecipient())
    {
        // normally forward
        processMsg(message.ref());
        return 0;
    }

    switch (message->id())
    {
        case CERBERUS_MESSAGE_TASK_ID:
            processTaskMsg(message.ref());
            break;
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

    initializeThreadPool();
}
//=============================================================================
void CerberusCore::coolDown()
{
    logInfo("Stopping event scheduler");
    m_eventScheduler.join(true);

    deinitializeThreadPool();

    logInfo("Stopping Cerberus core");
}
//=============================================================================
void CerberusCore::processTaskMsg(cerberus_message msg)
{
    auto tm = msg->getConstSlot("task")->to<TaskSlot>()->value();
    m_pool.runTask(tm);
}
//=============================================================================
void CerberusCore::processMsg(cerberus_message msg) { Cerberus::sendMsgToObj(msg->recipient(), msg.ref()); }
//=============================================================================
CerberusCore::CerberusCore()
    : cerberus::Thread(TP_Message, "Cerberus core"),
      m_pool()
{
}
//=============================================================================
CerberusCore::~CerberusCore() {}
//=============================================================================
void CerberusCore::setup(const CoreConf& parms) { m_conf = parms; }
//=============================================================================
