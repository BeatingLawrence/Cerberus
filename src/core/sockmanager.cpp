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
SockManager::~SockManager() {}
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
    MutexLocker ml(m_mutex);

    SockData *p = nullptr;

    for (auto &&el : m_sockets)
        if (el->handle == sock) p = el;

    if (!p) return OR_NotFound;

    MutexLocker ml2(p->mutex);

    // check if the thread is already in the list

    for (auto &&el : p->threads)
        if (el == threadID) return OR_AlreadyPresent;

    p->threads.push_back(threadID);

    return OR_OK;
}
//=============================================================================
SockManager::SockData *SockManager::addTcp(const SockSettings &settings, Socket *sock)
{
    if (sock == nullptr) return nullptr;

    auto p = new SockData(sock);

    p->settings = settings;

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
        if (el->handle == sock) return el;

    return OR_NotFound;
}
//=============================================================================
