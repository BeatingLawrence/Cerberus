#include "udpsocket.h"

#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../core/cerberuslog.h"

using namespace cerberus::socket;

//=============================================================================
UdpSocket::UdpSocket() : cerberus::socket::SocketBase(Socket_UDP)
{
    m_fd = ::socket(AF_INET, SOCK_DGRAM, 0);

    if (m_fd == -1)
    {
        debug("error in socket creation: %s", strerror(errno));
    }
}
//=============================================================================
UdpSocket::~UdpSocket() {}
//=============================================================================
cerberus::SocketOperation UdpSocket::sendTo(const data::ByteBuffer& buffer, const Host& dest, bool donotblock)
{
    if (isFailed())
    {
        return SocketOperation::SO_FailedSocket;
    }

    int flags = 0;

    if (donotblock)
    {
        flags |= MSG_DONTWAIT;
    }

    Host h = dest;

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(h.port);

    if (!h.hostname.empty())
    {
        resolve(h);
    }

    addr.sin_addr.s_addr = h.octet_networkOrder;

    auto ret = ::sendto(m_fd, buffer.data(), buffer.size(), flags, (sockaddr*)&addr, sizeof(sockaddr_in));

    if (ret == -1)
    {
        debug("socket send error, %s", strerror(errno));
        return SO_Failure;
    }

    return SO_OK;
}
//=============================================================================
