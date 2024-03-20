#include "cerberuscore.h"

#include "../cerberus.h"
#include "../network/socket.h"
#include "../thread/thread.h"

using namespace cerberus::core;

//=============================================================================
void CerberusCore::initializeThreadPool()
{
    if (m_conf.threadPool == 0) return;

    logInfo("Building thread pool");

    for (SIZE i = 0; i < m_conf.threadPool; i++)
    {
        auto t = new Actor(true);
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
}
//=============================================================================
void CerberusCore::taskEndCb(void* ctx, Actor* thread, OpRes res)
{
    ((CerberusCore*)ctx)->_taskEndCb(thread, res);
}
//=============================================================================
void CerberusCore::_taskEndCb(Actor* thread, OpRes res) { logInfo("Task end %s", res.reason.c_str()); }
//=============================================================================
int CerberusCore::tick()
{
    cerberus_message message = nextMessage();

    if (!(message->isValid())) return 0;

    // Process message queue..

    switch (message->id())
    {
        case CERBERUS_MESSAGE_SOCK_ID:
            processSockMsg(message.ref());
            break;

        case CERBERUS_MESSAGE_TASK_ID:
            processTaskMsg(message.ref());
            break;

        default:
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
void CerberusCore::processSockMsg(cerberus_message msg)
{
    if (msg->hasValidRecipient())
    {
        logError("specified an invalid socket ID");
        return;
    }

    auto obj = Cerberus::rawObjById(msg->recipient());

    if (obj.fail())
    {
        logError("specified socket ID cannot be found");
        return;
    }

    CerberusObject* target = obj.value;

    if (target->type() != COBJ_Socket)
    {
        logError("specified object type is not a socket");
        return;
    }

    Socket* sock = target->to_p<Socket>();

    if (!(sock->isConnected()))
    {
        sock->connect(msg->getConstSlot("host")->to<HostSlot>()->value());
    }

    sock->send(msg->getConstSlot("payload")->to<BufferSlot>()->value());
}
//=============================================================================
void CerberusCore::processTaskMsg(cerberus_message msg)
{
    if (msg->hasValidRecipient())
    {
        // normal mode
        auto obj = Cerberus::rawObjById(msg->recipient());

        if (obj.fail())
        {
            logError("specified thread ID cannot be found");
            return;
        }

        CerberusObject* target = obj.value;

        if (target->type() != COBJ_Thread)
        {
            logError("specified object type is not a thread");
            return;
        }

        Thread* act = target->to_p<Thread>();
        act->addMessage(msg.ref());

        return;
    }

    // thread pool mode

    auto tm = msg->getConstSlot("task")->to<TaskSlot>()->value();
    runTask(tm);
}
//=============================================================================
void CerberusCore::processMsg(cerberus_message msg)
{
    if (!(msg->hasValidRecipient()))
    {
        logWarning("Destination of message is invalid, dropping");
        return;
    }

    Cerberus::sendMsgToObj(msg->recipient(), msg.ref());
}
//=============================================================================
void CerberusCore::runTask(Task t)
{
    // find an available actor to assign the task to

    for (auto&& el : m_pool)
    {
        if (el->end())
        {
            el->run(t.cb, t.ctx).expect();
            return;
        }
    }

    throw cerberusIllegalStateExc("Could not assign a task to the thread pool, %u threads are not enough",
                                  m_conf.threadPool);
}
//=============================================================================
CerberusCore::CerberusCore()
    : cerberus::Thread(TP_Message, "Cerberus Core"),
      m_pool()
{
}
//=============================================================================
CerberusCore::~CerberusCore() {}
//=============================================================================
void CerberusCore::setup(const CoreConf& parms) { m_conf = parms; }
//=============================================================================
