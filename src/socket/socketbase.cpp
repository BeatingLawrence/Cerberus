#include "./socketbase.h"
#include "src/core/cerberuslog.h"
#include "src/data/bytebuffer.h"
#include <netdb.h>
#include <string.h>
#include <unistd.h>

//=============================================================================
cerberus::socket::SocketBase::SocketBase(SocketType type) : m_type(type) {}
//=============================================================================
cerberus::socket::SocketBase::~SocketBase()
{
    close();
}
//=============================================================================
bool cerberus::socket::SocketBase::isFailed() const
{
    return (m_fd == -1);
}
//=============================================================================
cerberus::SocketOperation cerberus::socket::SocketBase::resolve(Host& ip)
{
    addrinfo* res = nullptr;
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

    if(ret == 0)
    {
        sockaddr_in* addr = (sockaddr_in*)(res->ai_addr);
        ip.octet_networkOrder = addr->sin_addr.s_addr;
        freeaddrinfo(res);
        return SO_OK;
    }

    if(ret == EAI_AGAIN)
    {
        debug("DNS lookup: temporary server failure [%s]", ip.hostname.c_str());
        return SO_ResolveServerTempFailure;
    }
    else if(ret == EAI_FAIL)
    {
        debug("DNS lookup: server failure [%s]", ip.hostname.c_str());
        return SO_ResolveServerFailure;
    }
    else if(ret == EAI_NODATA)
    {
        debug("DNS lookup: hostname exists but has no ip associated [%s]", ip.hostname.c_str());
        return SO_ResolveNoData;
    }
    else if(ret == EAI_NONAME)
    {
        debug("DNS lookup: hostname was not found [%s]", ip.hostname.c_str());
        return SO_ResolveNotFound;
    }
    else if(ret == EAI_SYSTEM)
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
cerberus::SocketOperation cerberus::socket::SocketBase::bind(Host& interface)
{
    if(isFailed())
    {
        return SocketOperation::SO_FailedSocket;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(interface.port);

    if(!interface.hostname.empty())
    {
        resolve(interface);
    }

    addr.sin_addr.s_addr = interface.octet_networkOrder;
    int ret = ::bind(m_fd, (sockaddr*)&addr, sizeof(sockaddr_in));

    if(ret == -1)
    {
        debug("error in socket bind: %s", strerror(errno));
        return SO_BindFailure;
    }

    return SO_OK;
}
//=============================================================================
cerberus::SocketOperation cerberus::socket::SocketBase::connect(Host& dest)
{
    if(isFailed())
    {
        return SocketOperation::SO_FailedSocket;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(dest.port);

    if(!dest.hostname.empty())
    {
        resolve(dest);
    }

    addr.sin_addr.s_addr = dest.octet_networkOrder;
    int ret = ::connect(m_fd, (sockaddr*)&addr, sizeof(sockaddr_in));

    if(ret == -1)
    {
        debug("error in socket connect: %s", strerror(errno));
        return SO_ConnectFailure;
    }

    return SO_OK;
}
//=============================================================================
cerberus::SocketOperation cerberus::socket::SocketBase::close()
{
    if(::close(m_fd) == -1)
    {
        debug("socket close error, %s", strerror(errno));
        return SO_Failure;
    }

    return SO_OK;
}
//=============================================================================
cerberus::SocketOperation cerberus::socket::SocketBase::send(const data::ByteBuffer& buffer, bool donotblock)
{
    if(isFailed())
    {
        return SocketOperation::SO_FailedSocket;
    }

    int flags = 0;

    if(donotblock)
    {
        flags |= MSG_DONTWAIT;
    }

    auto ret = ::send(m_fd, buffer.data(), buffer.size(), flags);

    if(ret == -1)
    {
        debug("socket send error, %s", strerror(errno));
        return SO_Failure;
    }

    return SO_OK;
}
//=============================================================================
cerberus::SocketOperation cerberus::socket::SocketBase::recv(data::ByteBuffer& buffer, bool donotblock) //improve this method
{
    if(isFailed())
    {
        return SocketOperation::SO_FailedSocket;
    }

    int flags = 0;

    if(donotblock)
    {
        flags |= MSG_DONTWAIT;
    }
    else
    {
        flags |= MSG_WAITALL;
    }

    auto ret = ::recv(m_fd, buffer.data(), buffer.size(), flags);

    if(ret == -1)
    {
        debug("socket recv error, %s", strerror(errno));
        return SO_Failure;
    }

    return SO_OK;
}
//=============================================================================
