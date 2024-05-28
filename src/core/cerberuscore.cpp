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
OpRes CerberusCore::sockCB(void* ctx, void* data)
{
    auto sock = (SockManager::SockData*)data;
    OpRes res(OR_OK);

    while (true)
    {
        MutexLocker ml(sock->mutex);

        if (sock->status != SockManager::SockData::Using) break;

        if (sock->settings.server)
        {
            auto s = sock->sock->accept();

            if (s) CerberusCore::processClient((CerberusCore*)ctx, s, sock->settings);
            // maybe send a message also?
            else
            {
                logError("Accept failed");
                res = OR_Failure;
                break;
            }

            continue;
        }

        if (sock->settings.type == Socket_TCP && !sock->sock->isConnected())
        {
            res = sock->sock->connect(sock->settings.remote);
            if (res.fail("sock connect fail in player")) break;
        }

        if (sock->sock->canRead().ok())
        {
            ByteBuffer buf;

            if (sock->settings.transferMode == Transfer_Bytes)
            {
                res = sock->sock->recv(buf);
                if (res.fail("recv fail in player")) break;
            }
            else if (sock->settings.transferMode == Transfer_Time)
            {
                res = sock->sock->recv(buf, sock->settings.tout, sock->settings.cyctout);
                if (res.fail("cyclic recv fail in player")) break;
            }

            auto msg = Cerberus::constructMessage(CERBERUS_MESSAGE_SOCKDATA_ID);
            msg->getSlot("result")->to<ResultSlot>()->value(OR_OK);
            msg->getSlot("host")->to<HostSlot>()->value(sock->sock->remote());
            msg->getSlot("buffer")->to<BufferSlot>()->value(buf);

            if (sock->threads.size() == 1)
            {
                Cerberus::sendMsgToObj(sock->threads.front(), msg);  // shallow copy
                continue;
            }

            for (auto&& el : sock->threads)
            {
                Cerberus::sendMsgToObj(el, msg.duplicate());  // deep copy
            }
        }
    }

    if (sock->status == SockManager::SockData::Remove)
        // delete the socket
        ((CerberusCore*)ctx)->m_socks.removeSock(sock);
    else
        sock->status = SockManager::SockData::Halted;

    return res;
}
//=============================================================================
void CerberusCore::processClient(CerberusCore* ctx, Socket* sock, const SockSettings& settings)
{
    SockSettings set(settings);

    set.bind   = Host();
    set.server = false;

    auto sockdata = ctx->m_socks.addTcp(set, sock);

    sockdata->status = SockManager::SockData::Using;

    Task t = {};
    t.cb   = CerberusCore::sockCB;
    t.ctx  = ctx;
    t.data = sockdata;

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
OpResData<CHANDLE> CerberusCore::newSock(const SockSettings& settings)
{
    auto sock = m_socks.newSock(settings);

    if (sock.fail()) return sock;

    sock.value->sock->setRecvBufferSize(settings.maxpayload);

    if (settings.bind.isValid())
    {
        auto res = sock.value->sock->bind(settings.bind);
        if (res.fail())
        {
            m_socks.removeSock(sock.value);
            return res;
        }
    }

    if (settings.server)
    {
        auto res = sock.value->sock->listen(settings.maxconn);
        if (res.fail())
        {
            m_socks.removeSock(sock.value);
            return res;
        }
    }

    sock.value->status = SockManager::SockData::Using;

    Task t = {};
    t.cb   = CerberusCore::sockCB;
    t.ctx  = this;
    t.data = sock.value;

    m_pool.runTask(t);

    return OR_OK;
}
//=============================================================================
OpRes CerberusCore::addSockListener(CHANDLE sock, HASH32 threadID)
{
    return m_socks.addListener(sock, threadID);
}
//=============================================================================
OpRes CerberusCore::sockSend(CHANDLE sock, const ByteBuffer& buffer)
{
    auto s = m_socks.getSock(sock);

    if (s.fail()) return s;

    s.value->mutex.lock();

    auto res = s.value->sock->send(buffer);

    s.value->mutex.unlock();

    return res;
}
//=============================================================================
OpRes CerberusCore::removeSock(CHANDLE sock)
{
    auto s = m_socks.getSock(sock);

    if (s.fail()) return s;

    s.value->mutex.lock();

    s.value->status = SockManager::SockData::Remove;

    s.value->mutex.unlock();

    return OR_OK;
}
//=============================================================================
