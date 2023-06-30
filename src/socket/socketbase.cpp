#include "./socketbase.h"

#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include "src/core/cerberuslog.h"
#include "src/data/bytebuffer.h"

#define DEFAULT_RECV_BUFFER_SIZE 512

//=============================================================================
cerberus::socket::SocketBase::SocketBase(SocketType type) : m_type(type), m_fd(-1), m_recvBuffer(DEFAULT_RECV_BUFFER_SIZE) {}
//=============================================================================
cerberus::socket::SocketBase::SocketBase(SocketType type, int fd) : m_type(type), m_fd(fd), m_recvBuffer(DEFAULT_RECV_BUFFER_SIZE) {}
//=============================================================================
cerberus::socket::SocketBase::~SocketBase() { close(); }
//=============================================================================
bool cerberus::socket::SocketBase::isFailed() const { return (m_fd == -1); }
//=============================================================================
void cerberus::socket::SocketBase::setRecvBufferSize(size_t size) { m_recvBuffer.resize(size); }
//=============================================================================
cerberus::SocketOperation cerberus::socket::SocketBase::resolve(Host &ip)
{
    addrinfo *res = nullptr;
    addrinfo info;
    info.ai_family = AF_INET;
    info.ai_socktype = 0;
    info.ai_protocol = 0;
    info.ai_flags = 0;
    info.ai_addr = nullptr;
    info.ai_addrlen = 0;
    info.ai_canonname = nullptr;
    info.ai_next = nullptr;
    int ret = getaddrinfo(ip.hostname.c_str(), nullptr, &info, &res);

    if (ret == 0)
    {
        sockaddr_in *addr = (sockaddr_in *)(res->ai_addr);
        ip.octet_networkOrder = addr->sin_addr.s_addr;
        freeaddrinfo(res);
        return SO_OK;
    }

    if (ret == EAI_AGAIN)
    {
        debug("DNS lookup: temporary server failure [%s]", ip.hostname.c_str());
        return SO_ResolveServerTempFailure;
    }
    else if (ret == EAI_FAIL)
    {
        debug("DNS lookup: server failure [%s]", ip.hostname.c_str());
        return SO_ResolveServerFailure;
    }
    else if (ret == EAI_NODATA)
    {
        debug("DNS lookup: hostname exists but has no ip associated [%s]", ip.hostname.c_str());
        return SO_ResolveNoData;
    }
    else if (ret == EAI_NONAME)
    {
        debug("DNS lookup: hostname was not found [%s]", ip.hostname.c_str());
        return SO_ResolveNotFound;
    }
    else if (ret == EAI_SYSTEM)
    {
        debug("DNS lookup: system failure, %s [%s]", strerror(errno), ip.hostname.c_str());
        return SO_ResolveNotFound;
    }
    else
    {
        debug("DNS lookup: failure, %s [%s]", gai_strerror(ret), ip.hostname.c_str());
        return SO_ResolveFailure;
    }
}
//=============================================================================
cerberus::SocketOperation cerberus::socket::SocketBase::bind(const Host &interface)
{
    if (isFailed())
    {
        return SocketOperation::SO_FailedSocket;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(interface.port);

    addr.sin_addr.s_addr = interface.octet_networkOrder;
    int ret = ::bind(m_fd, (sockaddr *)&addr, sizeof(sockaddr_in));

    if (ret == -1)
    {
        debug("error in socket bind: %s", strerror(errno));
        return SO_BindFailure;
    }

    return SO_OK;
}
//=============================================================================
cerberus::SocketOperation cerberus::socket::SocketBase::connect(const Host &dest)
{
    if (isFailed())
    {
        return SocketOperation::SO_FailedSocket;
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
    int ret = ::connect(m_fd, (sockaddr *)&addr, sizeof(sockaddr_in));

    if (ret == -1)
    {
        debug("error in socket connect: %s", strerror(errno));
        return SO_ConnectFailure;
    }

    return SO_OK;
}
//=============================================================================
cerberus::SocketOperation cerberus::socket::SocketBase::close()
{
    if (::close(m_fd) == -1)
    {
        if (errno == EBADF)  // already close
        {
            return SO_OK;
        }
        debug("socket close error, %s", strerror(errno));
        return SO_Failure;
    }

    return SO_OK;
}
//=============================================================================
cerberus::SocketOperation cerberus::socket::SocketBase::send(const data::ByteBuffer &buffer, bool donotblock)
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

    auto ret = ::send(m_fd, buffer.data(), buffer.size(), flags);

    if (ret == -1)
    {
        debug("socket send error, %s", strerror(errno));
        return SO_Failure;
    }

    return SO_OK;
}
//=============================================================================
cerberus::SocketOperation cerberus::socket::SocketBase::recv(data::ByteBuffer &buffer, bool donotblock)
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
    else
    {
        flags |= MSG_WAITALL;
    }

    auto ret = ::recv(m_fd, m_recvBuffer.data(), m_recvBuffer.size(), flags);

    if (ret == -1)
    {
        debug("socket recv error, %s", strerror(errno));
        return SO_Failure;  // improve error handling
    }

    buffer = m_recvBuffer;
    buffer.resize(ret);

    return SO_OK;
}
//=============================================================================
