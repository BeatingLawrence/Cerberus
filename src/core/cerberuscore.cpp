#include "cerberuscore.h"

#include "../cerberus.h"
#include "../thread/thread.h"

using namespace cerberus::core;

//=============================================================================
void CerberusCore::initializeThreadPool()
{
    if (m_conf.threadPool == 0) return;

    logInfo("Building thread pool");

    for (SIZE i = 0; i < m_conf.threadPool; i++)
    {
        auto t = new Player(true, CerberusUtils::strPrint("Player[%u]", i));
        t->setTaskEndCB(CerberusCore::taskEndCb, this).expect();
        m_pool.push_back(t);
    }
}
//=============================================================================
void CerberusCore::deinitializeThreadPool()
{
    if (m_pool.empty()) return;

    logInfo("Deallocating thread pool");

    for (auto& el : m_pool)
    {
        el->join(true).expect();
        delete el;
    }

    for (auto& el : m_reservedPool)
    {
        el->join(true).expect();
        delete el;
    }
}
//=============================================================================
void CerberusCore::cleanupPlayer(Thread* t)
{
    for (auto it = m_reservedPool.begin(); it != m_reservedPool.end(); it++)
    {
        if ((*it) == t)
        {
            m_reservedPool.erase(it);
            t->join(true);
            delete t;
            break;
        }
    }
}
//=============================================================================
void CerberusCore::taskEndCb(void* ctx, Player* thread, OpRes res)
{
    ((CerberusCore*)ctx)->_taskEndCb(thread, res);
}
//=============================================================================
void CerberusCore::_taskEndCb(Player* thread, OpRes res) { logDebug("task end"); }
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

        case CERBERUS_MESSAGE_TASKEND_ID:
            if (message->recipient() == id())
            {
                // recipient is the Cerberus core
                auto player = (Player*)(message->getConstSlot("player")->to<VoidPSlot>()->value());
                cleanupPlayer(player);
            }
            else
                processMsg(message.ref());
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
    runTask(tm);
}
//=============================================================================
void CerberusCore::processMsg(cerberus_message msg) { Cerberus::sendMsgToObj(msg->recipient(), msg.ref()); }
//=============================================================================
void CerberusCore::runTask(Task t)
{
    // find an available player to assign the task to
    for (auto&& el : m_pool)
    {
        if (el->end())
        {
            el->run(t.cb, t.ctx).expect();
            return;
        }
    }

    // all players are busy, let's create a temporary one
    auto p = new Player;
    p->checkIn();
    p->run(t.cb, t.ctx, t.data).expect();
    m_reservedPool.push_back(p);

    logDebug("Thread pool busy. A temporary thread has been created");
}
//=============================================================================
CerberusCore::CerberusCore()
    : cerberus::Thread(TP_Message, "Cerberus core"),
      m_pool(),
      m_reservedPool()
{
}
//=============================================================================
CerberusCore::~CerberusCore() {}
//=============================================================================
void CerberusCore::setup(const CoreConf& parms) { m_conf = parms; }
//=============================================================================
