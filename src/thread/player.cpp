#include "player.h"

#include "../cerberus.h"

using namespace cerberus;

//=============================================================================
void Player::sendTaskEndMsg(HASH32 recipient, OpRes res)
{
    auto msg = Cerberus::constructMessage(CERBERUS_MESSAGE_TASKEND_ID);
    msg->getSlot("result")->to<ResultSlot>()->value(res);
    msg->getSlot("player")->to<VoidPSlot>()->value(this);
    Cerberus::send(msg, recipient);
}
//=============================================================================
int Player::tick()
{
    if (isQueueEmpty())
    {
        // standard mode
        if (m_cb)
        {
            auto res = m_cb(m_ctx, m_data);
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
    auto res = task.cb(task.ctx, task.data);

    sendTaskEndMsg(client, res);

    return 0;
}
//=============================================================================
Player::~Player() {}
//=============================================================================
Player::Player(bool manualTrigger, const std::string &name)
    : Thread(manualTrigger ? TP_Trigger : TP_Message, name),
      m_cb(nullptr),
      m_endCb(nullptr),
      m_ctx(nullptr),
      m_endCbCtx(nullptr)
{
}
//=============================================================================
bool Player::end() { return getPausedFlag(); }
//=============================================================================
bool Player::running() { return !end(); }
//=============================================================================
OpRes Player::start()
{
    if (running()) return OR_TemporaryUnavailable;
    Thread::start();
    return OR_OK;
}
//=============================================================================
OpRes Player::assign(playerCallback cb, void *ctx, void *data)
{
    if (running()) return OR_TemporaryUnavailable;
    m_cb   = cb;
    m_ctx  = ctx;
    m_data = data;
    return OR_OK;
}
//=============================================================================
OpRes Player::run(playerCallback cb, void *ctx, void *data)
{
    if (running()) return OR_TemporaryUnavailable;
    m_cb   = cb;
    m_ctx  = ctx;
    m_data = data;
    Thread::start();
    return OR_OK;
}
//=============================================================================
OpRes Player::setTaskEndCB(taskEndCallback cb, void *ctx)
{
    if (running()) return OR_TemporaryUnavailable;

    m_endCb    = cb;
    m_endCbCtx = ctx;
    return OR_OK;
}
//=============================================================================
