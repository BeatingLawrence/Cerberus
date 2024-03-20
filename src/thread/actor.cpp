#include "actor.h"

#include "../cerberus.h"

using namespace cerberus;

//=============================================================================
void Actor::sendTaskEndMsg(HASH32 recipient, OpRes res)
{
    auto msg = Cerberus::messageConstruct(CERBERUS_MESSAGE_TASKEND_ID);
    msg->getSlot("result")->to<ResultSlot>()->value(res);
    Cerberus::send(msg, recipient);
}
//=============================================================================
int Actor::tick()
{
    if (isQueueEmpty())
    {
        // standard mode
        if (m_cb)
        {
            auto res = m_cb(m_ctx);
            if (m_endCb)
            {
                m_endCb(m_endCbCtx, this, res);
            }
        }
        return 0;
    }

    // message mode
    auto msg = nextMessage();
    if (msg->id() != CERBERUS_MESSAGE_TASK_ID) return 0;

    auto client = msg->getSlot("client")->to<UInt64Slot>()->value();
    auto task   = msg->getSlot("task")->to<TaskSlot>()->value();

    if (!task.isValid()) return 0;

    // execute
    auto res = task.cb(task.ctx);

    sendTaskEndMsg(client, res);

    return 0;
}
//=============================================================================
Actor::~Actor() {}
//=============================================================================
Actor::Actor(bool manualTrigger, const std::string &name)
    : Thread(manualTrigger ? TP_Trigger : TP_Message, name),
      m_cb(nullptr),
      m_endCb(nullptr),
      m_ctx(nullptr),
      m_endCbCtx(nullptr)
{
}
//=============================================================================
bool Actor::end() { return getPausedFlag(); }
//=============================================================================
bool Actor::running() { return !end(); }
//=============================================================================
OpRes Actor::start()
{
    if (running()) return OR_TemporaryUnavailable;
    Thread::start();
    return OR_OK;
}
//=============================================================================
OpRes Actor::assign(actorCallback cb, void *ctx)
{
    if (running()) return OR_TemporaryUnavailable;
    m_cb  = cb;
    m_ctx = ctx;
    return OR_OK;
}
//=============================================================================
OpRes Actor::run(actorCallback cb, void *ctx)
{
    if (running()) return OR_TemporaryUnavailable;
    m_cb  = cb;
    m_ctx = ctx;
    Thread::start();
    return OR_OK;
}
//=============================================================================
OpRes Actor::setTaskEndCB(taskEndCallback cb, void *ctx)
{
    if (running()) return OR_TemporaryUnavailable;

    m_endCb    = cb;
    m_endCbCtx = ctx;
    return OR_OK;
}
//=============================================================================
