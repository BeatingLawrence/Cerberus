#include "tcpsocket.h"

#include <netdb.h>
#include <string.h>
#include <sys/socket.h>

#include "../core/cerberuslog.h"

using namespace cerberus::socket;

//=============================================================================
TcpSocket::TcpSocket(int fd) : cerberus::socket::SocketBase(Socket_TCP, fd), m_maxConnections(0) {}
//=============================================================================
TcpSocket::TcpSocket() : cerberus::socket::SocketBase(Socket_TCP), m_maxConnections(0)
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
    // if (::listen(m_fd, maxconn == 0 ? m_maxConnections : maxconn) == -1)
    if (::listen(m_fd, 100) == -1)
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
    socklen_t len = 0;

    int ret = ::accept(m_fd, (sockaddr *)&addr, &len);

    debug("%u : %u  %u", addr.sin_addr.s_addr, addr.sin_port, len);

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
