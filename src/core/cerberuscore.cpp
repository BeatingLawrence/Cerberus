#include "cerberuscore.h"

#include <iomanip>
#include <sstream>
#include <vector>

#include "../cerberus.h"
#include "../thread/thread.h"

using namespace crb::core;
using namespace crb;

//=============================================================================
static std::string idsToStr(const std::vector<HASH32>& ids)
{
    std::ostringstream ss;
    for (size_t i = 0; i < ids.size(); ++i)
    {
        if (i != 0) ss << ",";
        ss << "0x" << std::hex << std::setw(8) << std::setfill('0') << ids[i];
        ss << std::dec;
    }
    return ss.str();
}
//=============================================================================
void CerberusCore::initializeThreadPool()
{
    if (m_conf.threadPool == 0) return;

    tlogDebug("Building thread pool");

    m_pool.build(m_conf.threadPool, m_conf.backupThreadMaxTime);
}
//=============================================================================
void CerberusCore::deinitializeThreadPool()
{
    if (m_pool.size() == 0) return;

    tlogDebug("Deallocating thread pool");

    m_pool.clear();
}
//=============================================================================
int CerberusCore::tick()
{
    msg_ptr message;

    if (size(1) > 0)
        message = next(1);  // retry queue first to preserve per-recipient order
    else
        message = next();

    if (!message) return 0;

    // Process message queue..

    if (message->hasValidRecipient())
    {
        // normally forward
        processMsg(message);
        return 0;
    }

    switch (message->id())
    {
        case CRB_MESSAGE_TASK_ID:
            processTaskMsg(message);
            break;
    }

    // Do other stuff..
    return 0;
}
//=============================================================================
void CerberusCore::warmUp()
{
    tlogInfo("Starting Core Thread");

    tlogDebug("Starting signal handler");
    m_signalHandler.start();

    logDebug("Starting Event Scheduler");
    m_eventScheduler.start();

    initializeThreadPool();
}
//=============================================================================
void CerberusCore::coolDown()
{
    tlogDebug("Stopping signal handler");
    m_signalHandler.join(true);

    tlogDebug("Stopping event scheduler");
    m_eventScheduler.join(true);

    deinitializeThreadPool();

    tlogInfo("Stopping Core Thread");
}
//=============================================================================
void CerberusCore::processTaskMsg(msg_ptr& msg)
{
    auto tm = msg->getSlot("task")->to<TaskSlot>()->value();
    m_pool.runTask(tm);
}
//=============================================================================
void CerberusCore::processMsg(msg_ptr& msg)
{
    auto recipients = msg->recipients();

    for (size_t i = 0; i < recipients.size(); i++)
    {
        if (i == (recipients.size() - 1))  // last, avoid deep copy
        {
            msg->setRecipient(recipients[i]);
            if (sendMsgToObj(recipients[i], msg).fail())
            {
                requeueFront(msg, 1).expect("requeue failed");
            }
        }
        else  // not last, use deep copy
        {
            auto aux = msg.duplicate();
            if (!aux) continue;
            aux->setRecipient(recipients[i]);
            if (sendMsgToObj(recipients[i], aux).fail())
            {
                requeueFront(aux, 1).expect("requeue failed");
            }
        }
    }
}
//=============================================================================
CerberusCore::CerberusCore()
    : Thread(TP_Message)
{
    setThreadName("Core");
}
//=============================================================================
CerberusCore::~CerberusCore() {}
//=============================================================================
void CerberusCore::setup(const CoreConf& parms) { m_conf = parms; }
//=============================================================================
