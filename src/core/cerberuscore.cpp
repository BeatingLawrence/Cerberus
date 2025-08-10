#include "cerberuscore.h"

#include "../cerberus.h"
#include "../thread/thread.h"

using namespace cerberus::core;
using namespace cerberus;

//=============================================================================
void CerberusCore::initializeThreadPool()
{
    if (m_conf.threadPool == 0) return;

    logInfo("Building thread pool");

    m_pool.build(m_conf.threadPool, m_conf.backupThreadMaxTime);
}
//=============================================================================
void CerberusCore::deinitializeThreadPool()
{
    if (m_pool.size() == 0) return;

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

    logInfo("Starting Event Scheduler");
    m_eventScheduler.start();

    initializeThreadPool();
}
//=============================================================================
void CerberusCore::coolDown()
{
    logInfo("Stopping event scheduler");
    m_eventScheduler.join(true);

    logInfo("removing sockets");
    m_sockets.scheduleRemoval_all();

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
void CerberusCore::processMsg(cerberus_message msg) { Cerberus::sendMsgToObj(msg->recipient(), msg); }
//=============================================================================
OpRes CerberusCore::socketCB(void* ctx, void* data)
{
    auto socket = (SocketManager::SocketData*)data;
    OpRes res(OR_OK);

    while (true)
    {
        MutexLocker ml(socket->mutex);

        if (socket->remove) break;

        if (socket->settings.server)
        {
            if (socket->s->canRead().fail()) continue;

            auto s = socket->s->accept();

            if (s.get())
            {
                s.ref();  // increment ref count
                CerberusCore::processClient((CerberusCore*)ctx, s.get(), socket, socket->settings);
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

        if (socket->settings.type == Socket_TCP && !socket->s->isConnected())
        {
            if (!socket->settings.remote.isValidRemote()) break;  // abort

            res = socket->s->connect(socket->settings.remote);
            if (res.fail("socket connect fail in player")) break;
        }

        if (socket->s->canRead().ok())
        {
            ByteBuffer buf;

            if (socket->settings.transferMode == Transfer_Bytes)
            {
                res = socket->s->recv(buf);
                if (res.fail("recv fail in player")) break;
            }
            else if (socket->settings.transferMode == Transfer_Time)
            {
                res = socket->s->recv(buf, socket->settings.tout, socket->settings.cyctout);
                if (res.fail("cyclic recv fail in player")) break;
            }

            auto msg = Cerberus::constructMessage(CERBERUS_MESSAGE_SOCKETDATA_ID);
            msg->getSlot("result")->to<ResultSlot>()->value(OR_OK);
            msg->getSlot("host")->to<HostSlot>()->value(socket->s->remote());
            msg->getSlot("buffer")->to<BufferSlot>()->value(buf);

            if (socket->threads.size() == 1)
            {
                Cerberus::sendMsgToObj(socket->threads.front(), msg);  // shallow copy
                continue;
            }

            for (auto&& el : socket->threads)
            {
                Cerberus::sendMsgToObj(el, msg.duplicate());  // deep copy
            }
        }
    }

    // delete the socket
    ((CerberusCore*)ctx)->m_sockets.removeSocket(socket);

    return res;
}
//=============================================================================
void CerberusCore::processClient(CerberusCore* ctx, Socket* socket, SocketManager::SocketData* parentData,
                                 const SocketSettings& settings)
{
    SocketSettings set(settings);

    set.bind   = Host();
    set.remote = Host();
    set.server = false;

    auto sdata = ctx->m_sockets.newSocketCopy(set, socket, parentData);

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
