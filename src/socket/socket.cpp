#include "./socket.h"

#include <netdb.h>
#include <netinet/tcp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <string.h>
#include <sys/sendfile.h>
#include <unistd.h>

#include "src/core/cerberuslog.h"
#include "src/data/bytebuffer.h"
#include "src/data/filesystem/file.h"
#include "src/exception/exceptioncatalog.h"
#include "src/time/timer.h"

#define DEFAULT_RECV_BUFFER_SIZE 512
#define DEFAULT_MAX_CONNECTIONS 15

//=============================================================================
cerberus::socket::Socket::Socket(SocketType type, int fd, SSL_CTX *ctx)
    : CerberusObject(type),
      m_extern(true),
      m_maxConnections(DEFAULT_MAX_CONNECTIONS),
      m_fd(fd),
      m_recvBuffer(DEFAULT_RECV_BUFFER_SIZE),
      m_sslCtx(ctx),
      m_ssl(nullptr)
{
    if (ctx != nullptr)
    {
        if (SSL_CTX_up_ref(ctx) != 1)
        {
            throw cerberusSystemExc("could not increment reference counter of SSL CTX");
        }

        m_ssl = SSL_new(m_sslCtx);

        if (m_ssl == NULL)
        {
            SSL_CTX_free(m_sslCtx);
            // TODO check the error
            cerberusSystemExc("SSL object creation failed");
        }

        if (SSL_set_fd(m_ssl, m_fd) != 1)
        {
            SSL_free(m_ssl);
            SSL_CTX_free(m_sslCtx);
            // TODO check the error
            cerberusSystemExc("SSL association with kernel fd failed");
        }
    }
}
//=============================================================================
int cerberus::socket::Socket::_accept(Host &peer)
{
    if (m_extern)
    {
        return -1;
    }

    sockaddr_in addr{};
    socklen_t len = sizeof(sockaddr_in);

    int ret = ::accept(m_fd, (sockaddr *)&addr, &len);

    if (ret == -1)
    {
        debug("error in socket accept: %s", strerror(errno));
        return -1;
    }

    peer.octet_networkOrder = addr.sin_addr.s_addr;
    peer.port               = ntohs(addr.sin_port);

    if (m_ssl)  // TLS mode
    {
        if (SSL_accept(m_ssl) != 1)  // SSL handshaking
        {
            logError("SSL accept error");
            ::close(ret);
            return -1;  // invalidate the new socket
        }
    }

    return ret;
}
//=============================================================================
cerberus::socket::Socket::Socket(SocketType type, const std::string &name)
    : CerberusObject(type, name),
      m_extern(false),
      m_maxConnections(DEFAULT_MAX_CONNECTIONS),
      m_fd(-1),
      m_recvBuffer(DEFAULT_RECV_BUFFER_SIZE),
      m_sslCtx(nullptr),
      m_ssl(nullptr)
{
    switch (transportType())
    {
        case TCP:
            createTcpSocket();
            break;
        case UDP:
            createUdpSocket();
            break;
        case ICMP:
            throw cerberusImplMissExc("ICMP sockets not implemented yet");
            break;
        case IPC:
            throw cerberusImplMissExc("IPC sockets not implemented yet");
            break;
    }

    if (!isFailed()) debug("New %s", toObjStr().c_str());
}
//=============================================================================
void cerberus::socket::Socket::createUdpSocket()
{
    m_fd = ::socket(AF_INET, SOCK_DGRAM, 0);

    if (m_fd == -1)
    {
        debug("error in UDP socket creation: %s", strerror(errno));
    }
}
//=============================================================================
void cerberus::socket::Socket::createTcpSocket()
{
    m_fd = ::socket(AF_INET, SOCK_STREAM, 0);

    if (m_fd == -1)
    {
        debug("error in TCP socket creation: %s", strerror(errno));
    }
}
//=============================================================================
cerberus::socket::Socket::TransportType cerberus::socket::Socket::transportType()
{
    switch (socketType())
    {
        case CerberusObject::Socket_None:
            throw cerberusIllegalArgExc("None type cannot be used");
        case CerberusObject::Socket_UDP:
            return UDP;
        case CerberusObject::Socket_TCP:
            return TCP;
        case CerberusObject::Socket_TCPP2P:
            return TCP;
        case CerberusObject::Socket_HTTP:
            throw cerberusImplMissExc("HTTP sockets not implemented yet");
        case CerberusObject::Socket_WEB:
            throw cerberusImplMissExc("WEB sockets not implemented yet");
        case CerberusObject::Socket_FTP:
            return TCP;
        case CerberusObject::Socket_ICMP:
            return ICMP;
        case CerberusObject::Socket_IPC:
            return IPC;
    }

    throw cerberusImplMissExc("requested socket has not been implemented yet");
}
//=============================================================================
cerberus::socket::Socket::~Socket()
{
    if (m_ssl)
    {
        SSL_free(m_ssl);
    }

    close();

    if (m_sslCtx)
    {
        SSL_CTX_free(m_sslCtx);
    }
}
//=============================================================================
bool cerberus::socket::Socket::isFailed() const { return (m_fd == -1) || (socketType() == Socket_None); }
//=============================================================================
void cerberus::socket::Socket::setRecvBufferSize(size_t size) { m_recvBuffer.resize(size); }
//=============================================================================
void cerberus::socket::Socket::setMaxConnections(size_t maxconn) { m_maxConnections = maxconn; }
//=============================================================================
cerberus::OperationResult cerberus::socket::Socket::listen(size_t maxconn)
{
    if (isFailed())
    {
        return OR_FailedInstance;
    }

    if (m_extern || transportType() != TCP)
    {
        return OR_Unavailable;
    }

    if (::listen(m_fd, maxconn == 0 ? m_maxConnections : maxconn) == -1)
    {
        debug("error in socket listen: %s", strerror(errno));
        return OR_Failure;
    }

    return OR_OK;
}
//=============================================================================
cerberus::socket::Socket cerberus::socket::Socket::accept(Host &peer)
{
    if (isFailed() || transportType() != TCP)
    {
        return {Socket_None, -1};
    }

    auto fd = _accept(peer);

    if (fd == -1)
        return {Socket_None, fd};
    else
        return {socketType(), fd};
}
//=============================================================================
cerberus::socket::Socket cerberus::socket::Socket::accept()
{
    Host peer;  // mock
    return accept(peer);
}
//=============================================================================
cerberus::OperationResult cerberus::socket::Socket::bind(const Host &iface)
{
    if (isFailed())
    {
        return OR_FailedInstance;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(iface.port);

    addr.sin_addr.s_addr = iface.octet_networkOrder;
    int ret              = ::bind(m_fd, (sockaddr *)&addr, sizeof(sockaddr_in));

    if (ret == -1)
    {
        debug("error in socket bind: %s", strerror(errno));
        return OR_Failure;
    }

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::socket::Socket::connect(const Host &dest)
{
    if (isFailed())
    {
        return OR_FailedInstance;
    }

    Host h = dest;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(h.port);

    if (!h.hostname.empty() && !h.resolved)
    {
        h.resolve();
    }

    if (h.octet_networkOrder == 0)
    {
        addr.sin_addr.s_addr = INADDR_ANY;
    }
    else
    {
        addr.sin_addr.s_addr = h.octet_networkOrder;
    }

    int ret = ::connect(m_fd, (sockaddr *)&addr, sizeof(sockaddr_in));

    if (ret == -1)
    {
        debug("error in socket connect: %s", strerror(errno));
        return OR_Failure;
    }

    if (m_ssl)  // TLS mode
    {
        if (SSL_connect(m_ssl) != 1)  // start handshaking
        {
            // TODO check the error
            logError("SSL connect failed");
            return OR_Failure;
        }
    }

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::socket::Socket::close()
{
    if (isFailed())
    {
        return OR_FailedInstance;
    }

    if (::close(m_fd) == -1)
    {
        if (errno == EBADF)  // already close
        {
            return OR_OK;
        }
        debug("socket close error, %s", strerror(errno));
        return OR_Failure;
    }

    return OR_OK;
}

//=============================================================================
cerberus::OperationResult cerberus::socket::Socket::useTLS(const std::string &certfile, const std::string &keyfile)
{
    if (isFailed())
    {
        return OR_FailedInstance;
    }

    if (m_ssl)
    {
        SSL_free(m_ssl);
    }

    if (m_sslCtx)
    {
        SSL_CTX_free(m_sslCtx);
    }

    m_sslCtx = SSL_CTX_new(TLS_method());

    if (m_sslCtx == NULL)
    {
        // TODO check the error
        logError("SSL context creation failed");
        return OR_SystemFailure;
    }

    if (!certfile.empty())
    {
        if (SSL_CTX_use_certificate_file(m_sslCtx, certfile.c_str(), SSL_FILETYPE_PEM) != 1)
        {
            SSL_CTX_free(m_sslCtx);
            // TODO check the error
            logError("SSL certificate file setup failed");
            return OR_Failure;
        }
    }

    if (!keyfile.empty())
    {
        if (SSL_CTX_use_PrivateKey_file(m_sslCtx, keyfile.c_str(), SSL_FILETYPE_PEM) != 1)
        {
            SSL_CTX_free(m_sslCtx);
            // TODO check the error
            logError("SSL private key file setup failed");
            return OR_Failure;
        }
    }

    if (!certfile.empty() && !keyfile.empty())
    {
        if (SSL_CTX_check_private_key(m_sslCtx) != 1)
        {
            logError("SSL private key check failed");  // continue anyway..
        }
    }

    m_ssl = SSL_new(m_sslCtx);

    if (m_ssl == NULL)
    {
        SSL_CTX_free(m_sslCtx);
        // TODO check the error
        logError("SSL object creation failed");
        return OR_SystemFailure;
    }

    if (SSL_set_fd(m_ssl, m_fd) != 1)
    {
        SSL_free(m_ssl);
        SSL_CTX_free(m_sslCtx);
        // TODO check the error
        logError("SSL association with kernel fd failed");
        return OR_Failure;
    }

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::socket::Socket::sendTo(const data::ByteBuffer &buffer, const Host &dest, bool donotblock)
{
    if (isFailed())
    {
        return OR_FailedInstance;
    }

    if (transportType() != UDP)
    {
        return OR_Unavailable;
    }

    int flags = 0;

    if (donotblock)
    {
        flags |= MSG_DONTWAIT;
    }

    Host h = dest;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(h.port);

    if (!h.hostname.empty() && !h.resolved)
    {
        h.resolve();
    }

    if (h.octet_networkOrder == 0)
    {
        addr.sin_addr.s_addr = INADDR_ANY;
    }
    else
    {
        addr.sin_addr.s_addr = h.octet_networkOrder;
    }

    auto ret = ::sendto(m_fd, buffer.data(), buffer.size(), flags, (sockaddr *)&addr, sizeof(sockaddr_in));

    if (ret == -1)
    {
        logError("socket send error, %s", strerror(errno));
        return OR_Failure;
    }

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::socket::Socket::send(const data::ByteBuffer &buffer, bool donotblock)
{
    if (isFailed())
    {
        return OR_FailedInstance;
    }

    if (m_ssl)  // TLS mode
    {
        if (SSL_write(m_ssl, buffer.data(), buffer.size()) != 1)
        {
            // TODO check the error
            logError("SSL socket send error");
            return OR_Failure;
        }

        return OR_OK;
    }

    int flags = 0;

    if (donotblock)
    {
        flags |= MSG_DONTWAIT;
    }

    auto ret = ::send(m_fd, buffer.data(), buffer.size(), flags);

    if (ret == -1)
    {
        logError("socket send error, %s", strerror(errno));
        return OR_Failure;
    }

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::socket::Socket::recv(data::ByteBuffer &buffer, bool donotblock)
{
    buffer.clear();

    if (isFailed())
    {
        return OR_FailedInstance;
    }

    if (m_ssl)  // TLS mode
    {
        size_t readB = 0;
        if (SSL_read_ex(m_ssl, m_recvBuffer.data(), m_recvBuffer.size(), &readB) != 1)
        {
            // TODO check the error
            logError("SSL socket recv error");
            return OR_Failure;
        }

        buffer = m_recvBuffer;
        buffer.resize(readB);
        return OR_OK;
    }

    int flags = 0;

    if (donotblock)
    {
        flags |= MSG_DONTWAIT;
    }

    auto ret = ::recv(m_fd, m_recvBuffer.data(), m_recvBuffer.size(), flags);

    if (ret == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return OR_WouldBlock;  // no data available at the moment
        debug("socket recv error, %s", strerror(errno));
        return OR_Failure;  // improve error handling
    }

    if (ret == 0)
    {
        return OR_RecvZero;  // EOF, stream closed in case of a stream socket, or zero length datagram in case of a datagram socket
    }

    buffer = m_recvBuffer;
    buffer.resize(ret);

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::socket::Socket::recv(data::ByteBuffer &buffer, const time::Time &timeout)
{
    if (isFailed())
    {
        return OR_FailedInstance;
    }

    data::ByteBuffer b;

    if (!timeout.isValid())
    {
        return OR_WrongArgument;
    }

    time::Timer timer(timeout);

    if (timer.isFailed())
    {
        return OR_Failure;
    }

    OperationResult ret   = OR_Failure;
    bool timerStartedFlag = false;

    while (true)
    {
        ret = Socket::recv(b, true);

        switch (ret.res)
        {
            case OR_OK:

                buffer.append(b);

                if (timer.isRunning())
                {
                    timer.stop();
                    timerStartedFlag = false;
                }
                break;

            case OR_WouldBlock:
                if (!timer.isRunning())
                {
                    timer.start();
                    timerStartedFlag = true;
                }
                break;

            case OR_RecvZero:  // EOF
                return OR_OK;

            default:
                return ret;
        }

        if (timerStartedFlag && !timer.isRunning())
        {
            break;
        }
    }

    if (timerStartedFlag)
    {
        return OR_TimedOut;
    }

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::socket::Socket::setCork(bool cork)
{
    if (isFailed())
    {
        return OR_FailedInstance;
    }

    if (transportType() != TCP)
    {
        return OR_Unavailable;
    }

    int val = cork ? 1 : 0;

    if (setsockopt(m_fd, IPPROTO_TCP, TCP_CORK, &val, sizeof(val)) == -1)
    {
        debug("error in setCork function: %s", strerror(errno));
        return OR_Failure;  // improve error handling
    }

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::socket::Socket::useNagle(bool use)
{
    if (isFailed())
    {
        return OR_FailedInstance;
    }

    if (transportType() != TCP)
    {
        return OR_Unavailable;
    }

    int val = use ? 1 : 0;

    if (setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val)) == -1)
    {
        debug("error in useNagle function: %s", strerror(errno));
        return OR_Failure;  // improve error handling
    }

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::socket::Socket::setTimeout(uint32_t timeout)
{
    if (isFailed())
    {
        return OR_FailedInstance;
    }

    if (transportType() != TCP)
    {
        return OR_Unavailable;
    }

    unsigned int val = timeout;

    if (setsockopt(m_fd, IPPROTO_TCP, TCP_USER_TIMEOUT, &val, sizeof(val)) == -1)
    {
        debug("error in setTimeout function: %s", strerror(errno));
        return OR_Failure;  // improve error handling
    }

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::socket::Socket::useKeepAlive(bool use, int maxprobes, int idleTime, int interval)
{
    if (isFailed())
    {
        return OR_FailedInstance;
    }

    if (transportType() != TCP)
    {
        return OR_Unavailable;
    }

    auto ret = useKeepAlive(use);

    if (ret != OR_OK)
    {
        return ret;
    }

    if (!use)
    {
        return OR_OK;
    }

    int val = maxprobes;

    if (setsockopt(m_fd, IPPROTO_TCP, TCP_KEEPCNT, &val, sizeof(val)) == -1)
    {
        debug("error in useKeepAlive:TCP_KEEPCNT function: %s", strerror(errno));
        return OR_Failure;  // improve error handling
    }

    val = idleTime;

    if (setsockopt(m_fd, IPPROTO_TCP, TCP_KEEPIDLE, &val, sizeof(val)) == -1)
    {
        debug("error in useKeepAlive:TCP_KEEPIDLE function: %s", strerror(errno));
        return OR_Failure;  // improve error handling
    }

    val = interval;

    if (setsockopt(m_fd, IPPROTO_TCP, TCP_KEEPINTVL, &val, sizeof(val)) == -1)
    {
        debug("error in useKeepAlive:TCP_KEEPINTVL function: %s", strerror(errno));
        return OR_Failure;  // improve error handling
    }

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::socket::Socket::useKeepAlive(bool use)
{
    if (isFailed())
    {
        return OR_FailedInstance;
    }

    if (transportType() != TCP)
    {
        return OR_Unavailable;
    }

    int val = use ? 1 : 0;

    if (setsockopt(m_fd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val)) == -1)
    {
        debug("error in useKeepAlive:SO_KEEPALIVE function: %s", strerror(errno));
        return OR_Failure;  // improve error handling
    }

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::socket::Socket::send(const data::filesystem::File &file)  // use SSL_sendfile for TLS mode
{
    if (isFailed())
    {
        return OR_FailedInstance;
    }

    if (socketType() != Socket_FTP)
    {
        return OR_Unavailable;
    }

    auto ret = sendfile(m_fd, file.m_fd, NULL, file.size());  // you may have to call it more times, FIX this

    if (ret == -1)
    {
        debug("socket sendfile error, %s", strerror(errno));
        return OR_Failure;
    }

    if (ret != file.size())
    {
        debug("NOT ALL BYTES TRANSFERRED");
    }

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::socket::Socket::recv(data::filesystem::File &file, const time::Time &timeout)
{
    if (isFailed())
    {
        return OR_FailedInstance;
    }

    if (socketType() != Socket_FTP)
    {
        return OR_Unavailable;
    }

    data::ByteBuffer buffer;
    OperationResult ret;

    ret = recv(buffer, timeout);

    switch (ret.res)
    {
        case OR_OK:
            // noop
            break;

        case OR_TimedOut:

            if (file.size() == 0)
            {
                return OR_TimedOut;
            }
            break;

        default:
            return ret;
            break;
    }

    if (!file.write(buffer))
    {
        return OR_Failure;
    }

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::socket::Socket::connectP2P(const Host &dest, const time::Time &timeout)  // implement ssl here
{
    if (isFailed())
    {
        return OR_FailedInstance;
    }

    if (socketType() != Socket_TCPP2P)
    {
        return OR_Unavailable;
    }

    Host h = dest;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(h.port);

    if (!h.hostname.empty() && !h.resolved)
    {
        h.resolve();
    }

    if (h.octet_networkOrder == 0)
    {
        addr.sin_addr.s_addr = INADDR_ANY;
    }
    else
    {
        addr.sin_addr.s_addr = h.octet_networkOrder;
    }

    bool overrideFlag = false;
    time::Timer timer(timeout);

    if (timeout.isValid())
    {
        timer.start();
    }
    else
    {
        overrideFlag = true;
    }

    int ret = -1;

    while (timer.isRunning() || overrideFlag)
    {
        ret = ::connect(m_fd, (sockaddr *)&addr, sizeof(sockaddr_in));

        if (ret == 0 || (ret == -1 && errno != ECONNREFUSED))
        {
            break;
        }
    }

    if (ret == -1)
    {
        debug("error in socket connect: %s", strerror(errno));
        return OR_Failure;
    }

    return OR_OK;
}
//=============================================================================
