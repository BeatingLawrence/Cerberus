#include "sockmanager.h"

#include "../mutex/mutexlocker.h"
#include "cerberusutils.h"
using namespace cerberus;

//=============================================================================
SockManager::SockData *SockManager::createNewSocket(SocketType type)
{
    if (type != Socket_TCP && type != Socket_UDP) return nullptr;
    return new SockData(type);
}
//=============================================================================
SockManager::SockManager() {}
//=============================================================================
SockManager::~SockManager() { clear(); }
//=============================================================================
OpResData<SockManager::SockData *> SockManager::newSock(const SockSettings &settings)
{
    auto p = createNewSocket(settings.type);

    if (p == nullptr) return OR_Failure;

    p->settings = settings;

    MutexLocker ml(m_mutex);

    m_sockets.push_back(p);

    // compute digest basing on the unique memory address of the allocated structure
    CHANDLE h = CerberusUtils::hash_fnv1a(ByteBuffer((BYTE *)p, sizeof(void *)));
    p->handle = h;

    return p;
}
//=============================================================================
OpRes SockManager::removeSock(SockData *sock)
{
    MutexLocker ml(m_mutex);
    sock->mutex.lock();  // wait for other threads to finish using the socket
    sock->mutex.unlock();

    for (auto it = m_sockets.begin(); it != m_sockets.end(); it++)
        if (sock == (*it))
        {
            m_sockets.erase(it);
            delete sock;
            return OR_OK;
        }

    return OR_NotFound;
}
//=============================================================================
OpRes SockManager::addListener(CHANDLE sock, HASH32 threadID)
{
    auto s = getSock(sock);

    if (s.fail()) return s;

    // check if the thread is already in the list

    for (auto &&el : s.value->threads)
        if (el == threadID)
        {
            s.value->mutex.unlock();
            return OR_AlreadyPresent;
        }

    s.value->threads.push_back(threadID);
    s.value->mutex.unlock();

    return OR_OK;
}
//=============================================================================
SockManager::SockData *SockManager::newSockCopy(const SockSettings &settings, Socket *sock,
                                                SockData *parentData)
{
    if (sock == nullptr || parentData == nullptr) return nullptr;

    auto p = new SockData(sock);

    p->settings = settings;
    p->threads  = parentData->threads;

    MutexLocker ml(m_mutex);

    m_sockets.push_back(p);

    // compute digest basing on the unique memory address of the allocated structure
    CHANDLE h = CerberusUtils::hash_fnv1a(ByteBuffer((BYTE *)p, sizeof(void *)));
    p->handle = h;

    return p;
}
//=============================================================================
OpResData<SockManager::SockData *> SockManager::getSock(CHANDLE sock)
{
    MutexLocker ml(m_mutex);

    for (auto &&el : m_sockets)
        if (el->handle == sock)
        {
            el->mutex.lock();

            if (el->remove)
            {
                el->mutex.unlock();
                return OR_Unavailable;
            }

            return el;
        }

    return OR_NotFound;
}
//=============================================================================
OpRes SockManager::sockSend(CHANDLE sock, const ByteBuffer &buffer)
{
    auto s = getSock(sock);

    if (s.fail()) return s;

    auto res = s.value->sock->send(buffer);
    s.value->mutex.unlock();

    return res;
}
//=============================================================================
OpRes SockManager::scheduleRemoval(CHANDLE sock)
{
    auto s = getSock(sock);

    if (s.fail()) return s;

    s.value->remove = true;
    s.value->mutex.unlock();

    return OR_OK;
}
//=============================================================================
void SockManager::scheduleRemoval_all()
{
    MutexLocker ml(m_mutex);

    for (auto &&el : m_sockets)
    {
        el->mutex.lock();
        el->remove = true;
        el->mutex.unlock();
    }
}
//=============================================================================
void SockManager::clear()
{
    MutexLocker ml(m_mutex);

    for (auto &el : m_sockets) delete el;

    m_sockets.clear();
}
//=============================================================================
