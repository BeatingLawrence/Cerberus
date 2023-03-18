#include "udpsocket.h"
#include "../core/cerberuslog.h"
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>

using namespace cerberus::socket;

//=============================================================================
UdpSocket::UdpSocket() : cerberus::socket::SocketBase(Socket_UDP)
{
    m_fd = ::socket(AF_INET, SOCK_DGRAM, 0);

    if(m_fd == -1)
    {
        debug("error in socket creation: %s", strerror(errno));
    }
}
//=============================================================================
UdpSocket::~UdpSocket()
{
}
//=============================================================================
cerberus::SocketOperation UdpSocket::sendTo(const data::ByteBuffer& buffer, const Host& dest)
{
}
//=============================================================================
