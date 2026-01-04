#include "cerberuscore.h"

#include "../cerberus.h"
#include "../thread/thread.h"

#include <iomanip>
#include <sstream>
#include <vector>

using namespace cerberus::core;
using namespace cerberus;

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

    logDebug("Building thread pool");

    m_pool.build(m_conf.threadPool, m_conf.backupThreadMaxTime);
}
//=============================================================================
void CerberusCore::deinitializeThreadPool()
{
    if (m_pool.size() == 0) return;

    logDebug("Deallocating thread pool");

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
        case CERBERUS_MESSAGE_TASK_ID:
            processTaskMsg(message);
            break;
    }

    // Do other stuff..
    return 0;
}
//=============================================================================
void CerberusCore::warmUp()
{
    logInfo("Starting Core Thread");

    logDebug("Starting Event Scheduler");
    m_eventScheduler.start();

    initializeThreadPool();
}
//=============================================================================
void CerberusCore::coolDown()
{
    logDebug("Stopping event scheduler");
    m_eventScheduler.join(true);

    logDebug("removing sockets");
    m_sockets.scheduleRemoval_all();

    deinitializeThreadPool();

    logInfo("Stopping Core Thread");
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
OpRes CerberusCore::socketCB(void* ctx, void* data)
{
    auto sdata = static_cast<SocketManager::SocketData*>(data);
    OpRes res(OR_OK);

    while (true)
    {
        MutexLocker ml(sdata->mutex);

        if (sdata->remove) break;

        if (sdata->settings.server)
        {
            if (sdata->s->canRead().fail()) continue;

            auto s = sdata->s->accept();

            if (s)
            {
                // do not use s anymore !
                CerberusCore::processClient((CerberusCore*)ctx, std::move(s), sdata, sdata->settings);
                // maybe send a message also?
            }
            else
            {
                logError("Accept failed");
                res = OR_Failure;
                break;
            }

            continue;
        }

        if (sdata->settings.type == Socket_TCP && !sdata->s->isConnected())
        {
            if (!sdata->settings.remote.isValidRemote()) break;  // abort

            res = sdata->s->connect(sdata->settings.remote);
            if (res.fail("socket connect fail in player")) break;
        }

        if (sdata->s->canRead().ok())
        {
            ByteBuffer buf;

            if (sdata->settings.transferMode == Transfer_Bytes)
            {
                res = sdata->s->recv(buf);
                if (res.fail("recv fail in player")) break;
            }
            else if (sdata->settings.transferMode == Transfer_Time)
            {
                res = sdata->s->recv_cyc(buf, sdata->settings.tout, sdata->settings.cyctout);
                if (res.fail("cyclic recv fail in player")) break;
            }

            auto msg = Cerberus::constructMessage(CERBERUS_MESSAGE_SOCKETDATA_ID);
            msg->getSlot("result")->to<ResultSlot>()->value(OR_OK);
            msg->getSlot("host")->to<HostSlot>()->value(sdata->s->remote());
            msg->getSlot("buffer")->to<BufferSlot>()->value(buf);

            if (sdata->threads.size() == 1)
            {
                res = Cerberus::sendMsgToObj(sdata->threads.front(), msg);
                if (res != OR_OK) logWarning("socketCB: queue full, message rejected");
                continue;
            }

            bool warned = false;
            for (auto&& el : sdata->threads)
            {
                OpRes r = Cerberus::sendMsgToObj_deep(el, msg);
                if (r != OR_OK)
                {
                    res = OR_Failure;
                    if (!warned)
                    {
                        warned = true;
                        logWarning("socketCB: one or more listeners rejected the message");
                    }
                }
            }
        }
    }

    // delete the socket
    ((CerberusCore*)ctx)->m_sockets.removeSocket(sdata);

    return res;
}
//=============================================================================
void CerberusCore::processClient(CerberusCore* ctx, cerberus_socket socket,
                                 SocketManager::SocketData* parentData, const SocketSettings& settings)
{
    SocketSettings set(settings);

    set.bind   = Host();
    set.remote = Host();
    set.server = false;

    auto sdata = ctx->m_sockets.newSocketCopy(set, std::move(socket), parentData);

    Task t = {};
    t.cb   = CerberusCore::socketCB;
    t.ctx  = ctx;
    t.data = sdata;

    ctx->m_pool.runTask(t);
}
//=============================================================================
CerberusCore::CerberusCore()
    : Thread(TP_Message, "Cerberus core")
{
}
//=============================================================================
CerberusCore::~CerberusCore() {}
//=============================================================================
void CerberusCore::setup(const CoreConf& parms) { m_conf = parms; }
//=============================================================================
OpResData<CHANDLE> CerberusCore::newSocket(const SocketSettings& settings)
{
    auto socket = m_sockets.newSocket(settings);

    if (socket.fail()) return (OpRes)socket;

    socket.value->s->setRecvBufferSize(settings.maxpayload);

    if (settings.bind.isValid())
    {
        auto res = socket.value->s->bind(settings.bind);
        if (res.fail())
        {
            m_sockets.removeSocket(socket.value);
            return res;
        }
    }

    if (settings.server)
    {
        auto res = socket.value->s->listen(settings.maxconn);
        if (res.fail())
        {
            m_sockets.removeSocket(socket.value);
            return res;
        }
    }

    Task t = {};
    t.cb   = CerberusCore::socketCB;
    t.ctx  = this;
    t.data = socket.value;

    m_pool.runTask(t);

    return socket.value->handle;
}
//=============================================================================
OpRes CerberusCore::addSocketListener(CHANDLE socket, HASH32 threadID)
{
    return m_sockets.addListener(socket, threadID);
}
//=============================================================================
OpRes CerberusCore::socketSend(CHANDLE socket, const ByteBuffer& buffer)
{
    return m_sockets.socketSend(socket, buffer);
}
//=============================================================================
OpRes CerberusCore::removeSocket(CHANDLE socket) { return m_sockets.scheduleRemoval(socket); }
//=============================================================================
