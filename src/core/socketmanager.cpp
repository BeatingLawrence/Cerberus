#include "socketmanager.h"

#include "../thread/mutexlocker.h"
#include "cerberusutils.h"
using namespace cerberus;

//=============================================================================
SocketManager::SocketData *SocketManager::createNewSocket(SocketType type)
{
    if (type != Socket_TCP && type != Socket_UDP) return nullptr;
    return new SocketData(type);
}
//=============================================================================
SocketManager::SocketManager() {}
//=============================================================================
SocketManager::~SocketManager() { clear(); }
//=============================================================================
OpResData<SocketManager::SocketData *> SocketManager::newSocket(const SocketSettings &settings)
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
OpRes SocketManager::removeSocket(SocketData *socket)
{
    MutexLocker ml(m_mutex);
    socket->mutex.lock();  // wait for other threads to finish using the socket
    socket->mutex.unlock();

    for (auto it = m_sockets.begin(); it != m_sockets.end(); it++)
        if (socket == (*it))
        {
            m_sockets.erase(it);
            delete socket;
            return OR_OK;
        }

    return OR_NotFound;
}
//=============================================================================
OpRes SocketManager::addListener(CHANDLE socket, HASH32 threadID)
{
    auto s = getSocket(socket);

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
SocketManager::SocketData *SocketManager::newSocketCopy(const SocketSettings &settings, Socket *socket,
                                                        SocketData *parentData)
{
    if (socket == nullptr || parentData == nullptr) return nullptr;

    auto p = new SocketData(socket);

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
OpResData<SocketManager::SocketData *> SocketManager::getSocket(CHANDLE socket)
{
    MutexLocker ml(m_mutex);

    for (auto &&el : m_sockets)
        if (el->handle == socket)
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
OpRes SocketManager::socketSend(CHANDLE socket, const ByteBuffer &buffer)
{
    auto s = getSocket(socket);

    if (s.fail()) return s;

    auto res = s.value->s->send(buffer);
    s.value->mutex.unlock();

    return res;
}
//=============================================================================
OpRes SocketManager::scheduleRemoval(CHANDLE socket)
{
    auto s = getSocket(socket);

    if (s.fail()) return s;

    s.value->remove = true;
    s.value->mutex.unlock();

    return OR_OK;
}
//=============================================================================
void SocketManager::scheduleRemoval_all()
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
void SocketManager::clear()
{
    MutexLocker ml(m_mutex);

    for (auto &el : m_sockets) delete el;

    m_sockets.clear();
}
//=============================================================================
