#include "tcpsocket.h"

#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/socket.h>

#include "../core/cerberuslog.h"

#define DEFAULT_MAX_CONNECTIONS 15

using namespace cerberus::socket;

//=============================================================================
TcpSocket::TcpSocket(int fd) : cerberus::socket::SocketBase(Socket_TCP, fd), m_maxConnections(DEFAULT_MAX_CONNECTIONS)
{
    if (m_fd == -1)
    {
        debug("error in socket creation: %s", strerror(errno));
    }
}
//=============================================================================
TcpSocket::TcpSocket() : cerberus::socket::SocketBase(Socket_TCP), m_maxConnections(DEFAULT_MAX_CONNECTIONS)
{
    m_fd = ::socket(AF_INET, SOCK_STREAM, 0);

    if (m_fd == -1)
    {
        debug("error in socket creation: %s", strerror(errno));
    }
}
//=============================================================================
void TcpSocket::setMaxConnections(size_t maxconn) { m_maxConnections = maxconn; }
//=============================================================================
cerberus::SocketOperation TcpSocket::listen(size_t maxconn)
{
    if (::listen(m_fd, maxconn == 0 ? m_maxConnections : maxconn) == -1)
    {
        debug("error in socket listen: %s", strerror(errno));
        return SO_ListenFailure;
    }

    return SO_OK;
}
//=============================================================================
TcpSocket TcpSocket::accept(Host &peer)
{
    sockaddr_in addr;
    socklen_t len = sizeof(sockaddr_in);

    int ret = ::accept(m_fd, (sockaddr *)&addr, &len);

    if (ret == -1)
    {
        debug("error in socket accept: %s", strerror(errno));
        return TcpSocket(-1);
    }

    peer.octet_networkOrder = addr.sin_addr.s_addr;
    peer.port = ntohs(addr.sin_port);
    return TcpSocket(ret);
}
//=============================================================================
TcpSocket TcpSocket::accept()
{
    int ret = ::accept(m_fd, NULL, NULL);

    if (ret == -1)
    {
        debug("error in socket accept: %s", strerror(errno));
        return TcpSocket(-1);
    }

    return TcpSocket(ret);
}
//=============================================================================
cerberus::SocketOperation TcpSocket::setCork(bool cork)
{
    int val = cork ? 1 : 0;

    if (setsockopt(m_fd, IPPROTO_TCP, TCP_CORK, &val, sizeof(val)) == -1)
    {
        debug("error in setCork function: %s", strerror(errno));
        return SO_Failure;  // improve error handling
    }

    return SO_OK;
}
//=============================================================================
cerberus::SocketOperation TcpSocket::useNagle(bool use)
{
    int val = use ? 1 : 0;

    if (setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val)) == -1)
    {
        debug("error in useNagle function: %s", strerror(errno));
        return SO_Failure;  // improve error handling
    }

    return SO_OK;
}
//=============================================================================
cerberus::SocketOperation TcpSocket::setTimeout(uint32_t timeout)
{
    unsigned int val = timeout;

    if (setsockopt(m_fd, IPPROTO_TCP, TCP_USER_TIMEOUT, &val, sizeof(val)) == -1)
    {
        debug("error in setTimeout function: %s", strerror(errno));
        return SO_Failure;  // improve error handling
    }

    return SO_OK;
}
//=============================================================================
cerberus::SocketOperation TcpSocket::useKeepAlive(bool use, int maxprobes, int idleTime, int interval)
{
    auto ret = useKeepAlive(use);

    if (ret != SO_OK)
    {
        return ret;
    }

    if (!use)
    {
        return SO_OK;
    }

    int val = maxprobes;

    if (setsockopt(m_fd, IPPROTO_TCP, TCP_KEEPCNT, &val, sizeof(val)) == -1)
    {
        debug("error in useKeepAlive:TCP_KEEPCNT function: %s", strerror(errno));
        return SO_Failure;  // improve error handling
    }

    val = idleTime;

    if (setsockopt(m_fd, IPPROTO_TCP, TCP_KEEPIDLE, &val, sizeof(val)) == -1)
    {
        debug("error in useKeepAlive:TCP_KEEPIDLE function: %s", strerror(errno));
        return SO_Failure;  // improve error handling
    }

    val = interval;

    if (setsockopt(m_fd, IPPROTO_TCP, TCP_KEEPINTVL, &val, sizeof(val)) == -1)
    {
        debug("error in useKeepAlive:TCP_KEEPINTVL function: %s", strerror(errno));
        return SO_Failure;  // improve error handling
    }

    return SO_OK;
}
//=============================================================================
cerberus::SocketOperation TcpSocket::useKeepAlive(bool use)
{
    int val = use ? 1 : 0;

    if (setsockopt(m_fd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val)) == -1)
    {
        debug("error in useKeepAlive:SO_KEEPALIVE function: %s", strerror(errno));
        return SO_Failure;  // improve error handling
    }

    return SO_OK;
}
//=============================================================================
