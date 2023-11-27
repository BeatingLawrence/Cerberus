#include "./socket.h"

#include <netdb.h>
#include <netinet/tcp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <poll.h>
#include <src/thread/thread.h>
#include <string.h>

#include "src/time/timer.h"
#ifdef LINUX_SYSTEM
#include <sys/sendfile.h>
#endif
#include <unistd.h>

#include "src/core/cerberuslog.h"
#include "src/data/filesystem/file.h"
#include "src/exception/exceptioncatalog.h"
#include "src/time/timer.h"

#define DEFAULT_RECV_BUFFER_SIZE 512
#define DEFAULT_MAX_CONNECTIONS 15

//=============================================================================
cerberus::network::Socket::Socket(SocketType type, int fd, SSL_CTX *ctx)
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
            throw cerberusSystemExc("SSL object creation failed");
        }

        if (SSL_set_fd(m_ssl, m_fd) != 1)
        {
            SSL_free(m_ssl);
            SSL_CTX_free(m_sslCtx);
            // TODO check the error
            throw cerberusSystemExc("SSL association with kernel fd failed");
        }
    }

    registerThis();
}
//=============================================================================
void cerberus::network::Socket::createUdpSocket()
{
    m_fd = ::socket(AF_INET, SOCK_DGRAM, 0);

    if (m_fd == -1)
    {
        cdebug("error in UDP socket creation: %s", strerror(errno));
    }
}
//=============================================================================
void cerberus::network::Socket::createTcpSocket()
{
    m_fd = ::socket(AF_INET, SOCK_STREAM, 0);

    if (m_fd == -1)
    {
        clogError("error in TCP socket creation: %s", strerror(errno));
        return;
    }

    int val = 1;
    if (::setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int)) == -1)
    {
        clogError("error in setsockopt() when setting SO_REUSEADDR: %s", strerror(errno));
        close();
    }
}
//=============================================================================
bool cerberus::network::Socket::reopen()
{
    close();
    createTcpSocket();  // recreate the socket

    if (m_bind.isValid())
    {
        // re-bind
        return bind(m_bind).ok();
    }

    return m_fd != -1;
}
//=============================================================================
void cerberus::network::Socket::recvUntilEOF(data::ByteBuffer &buffer)
{
    data::ByteBuffer b;

    while (true)
    {
        if (Socket::_recv(b, true).res == OR_OK)
            buffer += b;
        else
            return;
    }
}
//=============================================================================
cerberus::network::Socket::TransportType cerberus::network::Socket::transportType()
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
        case CerberusObject::Socket_ICMP:
            return ICMP;
        case CerberusObject::Socket_IPC:
            return IPC;
    }

    throw cerberusImplMissExc("requested socket has not been implemented yet");
}
//=============================================================================
int cerberus::network::Socket::_accept(Host &peer)
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
        cdebug("error in socket accept: %s", strerror(errno));
        return -1;
    }

    peer.octet_networkOrder = addr.sin_addr.s_addr;
    peer.port               = ntohs(addr.sin_port);

    if (isTLS())  // TLS mode
    {
        auto acc = TLS_accept();

        if (acc.fail())
        {
            ::close(ret);
            return -1;  // invalidate the new socket
        }
    }

    return ret;
}
//=============================================================================
void cerberus::network::Socket::printSSLErrors()
{
    char errstr[256] = {};

    unsigned long err = ERR_get_error();

    while (err != 0)
    {
        ERR_error_string_n(err, &errstr[0], sizeof(errstr));

        clogError("SSL: %s", &errstr);

        err = ERR_get_error();
    }
}
//=============================================================================
cerberus::OperationResult cerberus::network::Socket::_recv(data::ByteBuffer &buffer, bool donotblock)
{
    if (isFailed())
    {
        return OR_FailedInstance;
    }

    buffer.clear();

    if (isTLS())  // TLS mode
    {
        ERR_clear_error();

        size_t readB = 0;
        int ret      = SSL_read_ex(m_ssl, m_recvBuffer.data(), m_recvBuffer.size(), &readB);

        if (ret != 1)
        {
            auto err = SSL_get_error(m_ssl, ret);

            if (err == SSL_ERROR_ZERO_RETURN)  // EOF, shutdown
            {
                return OR_RecvZero;
            }
            else if (err == SSL_ERROR_WANT_READ)
            {
                return OR_TemporaryUnavailable;
            }

            // TODO check the error
            clogError("SSL socket recv error");
            printSSLErrors();
            return OR_Failure;
        }
        buffer.assign(m_recvBuffer, readB);
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
        cdebug("socket recv error, %s", strerror(errno));
        return OR_Failure;  // improve error handling
    }

    if (ret == 0)
    {
        return OR_RecvZero;  // EOF, stream closed in case of a stream socket, or zero length datagram in case of a datagram socket
    }

    buffer.assign(m_recvBuffer, ret);

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::network::Socket::TLS_connect()
{
    ERR_clear_error();

    if (SSL_connect(m_ssl) != 1)  // start SSL client handshaking
    {
        clogError("SSL connect error");
        printSSLErrors();
        return OR_Failure;
    }

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::network::Socket::TLS_accept()
{
    ERR_clear_error();

    if (SSL_accept(m_ssl) != 1)  // start SSL server handshaking
    {
        clogError("SSL accept error");
        printSSLErrors();
        return OR_Failure;
    }

    return OR_OK;
}
//=============================================================================
cerberus::network::Socket::Socket(SocketType type, const std::string &name)
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

    registerThis();

    if (!isFailed()) cdebug("New %s", toObjStr().c_str());
}
//=============================================================================
cerberus::network::Socket::~Socket()
{
    unregisterThis();

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
cerberus::OperationResult cerberus::network::Socket::bind(const Host &iface)
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
        clogError("error in socket bind: %s", strerror(errno));
        return OR_Failure;
    }

    m_bind = iface;  // backup iface

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::network::Socket::connect(const Host &dest)
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
        clogError("error in socket connect: %s", strerror(errno));

        if (transportType() == TCP)
        {
            close();  // the socket will not be usable anymore
        }

        return OR_Failure;
    }

    if (isTLS())  // TLS mode
    {
        return TLS_connect();
    }

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::network::Socket::send(const data::ByteBuffer &buffer, bool donotblock)
{
    if (isFailed())
    {
        return OR_FailedInstance;
    }

    if (isTLS())  // TLS mode
    {
        ERR_clear_error();

        int ret = SSL_write(m_ssl, buffer.data(), buffer.size());

        if (ret <= 0 || ret != buffer.size())
        {
            // TODO check the error
            clogError("SSL socket send error");
            printSSLErrors();
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
        clogError("socket send error, %s", strerror(errno));
        return OR_Failure;
    }

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::network::Socket::recv(data::ByteBuffer &buffer, const time::TimeFrame &timeout, const time::TimeFrame &cycTimeout)
{
    if (isFailed())
    {
        return OR_FailedInstance;
    }

    if (!timeout.isValid())
    {
        return _recv(buffer, true);
    }

    data::ByteBuffer b;
    buffer.clear();
    bool first = true;

    while (true)
    {
        auto waitRes = waitRead((first || !cycTimeout.isValid()) ? timeout : cycTimeout);  // for TLS must call read() to make stuff be ready
        first        = false;

        if (waitRes.res == OR_Hangup)  // maybe there's still some data to read
        {
            recvUntilEOF(buffer);
            break;
        }
        else if (waitRes.res == OR_TimedOut)
        {
            if (isTLS())  // maybe there's still some data remained in the TLS layer
            {
                while (TLS_pending().i != 0)
                {
                    if (Socket::_recv(b, false).ok())
                    {
                        buffer += b;
                    }
                    // else
                    //     break;    //maybe needed, maybe not
                }
            }

            break;
        }
        else if (waitRes.fail())
            return OR_Failure;

        // wait is OK, data is available

        auto ret = Socket::_recv(b, false).res;

        if (ret == OR_OK)
            buffer += b;
        else if (ret == OR_RecvZero)  // HUP or zero lenght datagram
            return OR_OK;
        else
            return ret;
    }

    // enough..
    if (buffer.size() == 0) return OR_TimedOut;

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::network::Socket::recv(data::ByteBuffer &buffer) { return _recv(buffer, false); }
//=============================================================================
void cerberus::network::Socket::setMaxConnections(size_t maxconn) { m_maxConnections = maxconn; }
//=============================================================================
bool cerberus::network::Socket::isFailed() const { return (m_fd == -1) || (socketType() == Socket_None); }
//=============================================================================
void cerberus::network::Socket::setRecvBufferSize(size_t size) { m_recvBuffer.resize(size); }
//=============================================================================
cerberus::OperationResult cerberus::network::Socket::waitRead(const time::TimeFrame &timeout)
{
    pollfd set{};
    set.fd     = m_fd;
    set.events = POLLIN;

    int ret = poll(&set, 1, timeout.isValid() ? timeout.toMilliseconds() : -1);

    if (ret == 0)  // timeout
    {
        return OR_TimedOut;
    }

    if (ret == -1)
    {
        clogError("error in poll: %s", strerror(errno));
        return OR_SystemFailure;
    }

    if (set.revents & POLLERR)
    {
        return OR_Failure;
    }

    if (set.revents & POLLHUP)
    {
        return OR_Hangup;
    }

    if (set.revents & POLLNVAL)
    {
        return OR_BadConditions;
    }

    if (set.revents & POLLIN)
    {
        return OR_OK;
    }

    return OR_Failure;
}
//=============================================================================
cerberus::OperationResult cerberus::network::Socket::waitWrite(const time::TimeFrame &timeout)
{
    pollfd set{};
    set.fd     = m_fd;
    set.events = POLLOUT;

    int ret = poll(&set, 1, timeout.isValid() ? timeout.toMilliseconds() : -1);

    if (ret == 0)  // timeout
    {
        return OR_TimedOut;
    }

    if (ret == -1)
    {
        clogError("error in poll: %s", strerror(errno));
        return OR_SystemFailure;
    }

    if (set.revents & POLLERR)
    {
        return OR_Failure;
    }

    if (set.revents & POLLHUP)
    {
        return OR_Hangup;
    }

    if (set.revents & POLLNVAL)
    {
        return OR_BadConditions;
    }

    if (set.revents & POLLOUT)
    {
        return OR_OK;
    }

    return OR_Failure;
}
//=============================================================================
cerberus::OperationResult cerberus::network::Socket::close()
{
    if (isFailed())
    {
        return OR_FailedInstance;
    }

    TLS_deinit();  // discard return value

    shutdown(m_fd, SHUT_WR);  // discard return value

    Result res = OR_OK;

    if (::close(m_fd) == -1)
    {
        clogError("socket close error, %s", strerror(errno));
        res = OR_Failure;
    }

    m_fd = -1;

    return res;
}
//=============================================================================
bool cerberus::network::Socket::isTLS() { return (m_sslCtx) && (m_ssl); }
//=============================================================================
cerberus::OperationResult cerberus::network::Socket::TLS_init(const std::string &certfile, const std::string &keyfile, bool forceServer)
{
    if (isFailed())
    {
        return OR_FailedInstance;
    }

    if (transportType() != TCP)
    {
        return OR_Unavailable;
    }

    ERR_clear_error();

    if (m_ssl)
    {
        SSL_free(m_ssl);
        m_ssl = nullptr;
    }

    if (m_sslCtx)
    {
        SSL_CTX_free(m_sslCtx);
    }

    m_sslCtx = SSL_CTX_new(forceServer ? TLS_server_method() : TLS_method());

    if (m_sslCtx == NULL)
    {
        // TODO check the error
        clogError("SSL context creation failed");
        printSSLErrors();
        return OR_SystemFailure;
    }

    if (!certfile.empty())
    {
        if (SSL_CTX_use_certificate_file(m_sslCtx, certfile.c_str(), SSL_FILETYPE_PEM) != 1)
        {
            SSL_CTX_free(m_sslCtx);
            // TODO check the error
            clogError("SSL certificate file setup failed");
            printSSLErrors();
            return OR_Failure;
        }
    }

    if (!keyfile.empty())
    {
        if (SSL_CTX_use_PrivateKey_file(m_sslCtx, keyfile.c_str(), SSL_FILETYPE_PEM) != 1)
        {
            SSL_CTX_free(m_sslCtx);
            // TODO check the error
            clogError("SSL private key file setup failed");
            printSSLErrors();
            printSSLErrors();
            return OR_Failure;
        }
    }

    if (!certfile.empty() && !keyfile.empty())
    {
        if (SSL_CTX_check_private_key(m_sslCtx) != 1)
        {
            clogError("SSL private key check failed");  // continue anyway..
            printSSLErrors();
        }
    }

    ERR_clear_error();

    m_ssl = SSL_new(m_sslCtx);

    if (m_ssl == NULL)
    {
        SSL_CTX_free(m_sslCtx);
        // TODO check the error
        clogError("SSL object creation failed");
        printSSLErrors();
        return OR_SystemFailure;
    }

    if (SSL_set_fd(m_ssl, m_fd) != 1)
    {
        SSL_free(m_ssl);
        SSL_CTX_free(m_sslCtx);
        // TODO check the error
        clogError("SSL association with kernel fd failed");
        printSSLErrors();
        return OR_Failure;
    }

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::network::Socket::TLS_deinit()
{
    if (!isTLS()) return OR_Unavailable;

    SSL_free(m_ssl);
    SSL_CTX_free(m_sslCtx);  // decrement reference counter
    m_ssl    = nullptr;
    m_sslCtx = nullptr;

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::network::Socket::TLS_shutdown()
{
    if (!isTLS()) return OR_Unavailable;

    // TLS mode

    ERR_clear_error();

    int ret = SSL_shutdown(m_ssl);

    if (ret == 0)  // unidirectional shutdown completed
    {
        return (int64_t)0;
    }
    else if (ret == 1)  // bidirectional shutdown completed
    {
        return (int64_t)1;
    }
    else  // error
    {
        // TODO check the error
        clogError("SSL socket shutdown error");
        printSSLErrors();
        return OR_Failure;
    }
}
//=============================================================================
cerberus::OperationResult cerberus::network::Socket::TLS_ignoreHangup(bool restrict)
{
    if (!isTLS()) return OR_Unavailable;

    // TLS mode

    SSL_set_options(m_ssl, SSL_OP_IGNORE_UNEXPECTED_EOF);

    if (!restrict)
    {
        SSL_CTX_set_options(m_sslCtx, SSL_OP_IGNORE_UNEXPECTED_EOF);
    }

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::network::Socket::TLS_getShutdown()
{
    if (!isTLS()) return OR_Unavailable;

    // TLS mode

    int ret = SSL_get_shutdown(m_ssl);

    if ((ret & SSL_SENT_SHUTDOWN) == SSL_SENT_SHUTDOWN)
    {
        if ((ret & SSL_RECEIVED_SHUTDOWN) == SSL_RECEIVED_SHUTDOWN)
            return (int64_t)3;
        else
            return (int64_t)1;
    }
    else
    {
        if ((ret & SSL_RECEIVED_SHUTDOWN) == SSL_RECEIVED_SHUTDOWN)
            return (int64_t)2;
        else
            return (int64_t)0;
    }
}
//=============================================================================
std::string cerberus::network::Socket::TLS_getProtocolName()
{
    if (isTLS())
        return SSL_get_cipher_version(m_ssl);
    else
        return "";
}
//=============================================================================
std::string cerberus::network::Socket::TLS_getCipherName()
{
    if (isTLS())
        return SSL_get_cipher_name(m_ssl);
    else
        return "";
}
//=============================================================================
cerberus::OperationResult cerberus::network::Socket::TLS_securePeerRenegSupport()
{
    if (!isTLS()) return OR_Unavailable;

    // TLS mode

    if (SSL_get_secure_renegotiation_support(m_ssl) == 1)
    {
        return (int64_t)1;
    }

    return (int64_t)0;
}
//=============================================================================
cerberus::OperationResult cerberus::network::Socket::TLS_hasPending()
{
    if (!isTLS()) return OR_Unavailable;

    // TLS mode

    if (SSL_has_pending(m_ssl) == 1) return (int64_t)1;

    return (int64_t)0;
}
//=============================================================================
cerberus::OperationResult cerberus::network::Socket::TLS_pending()
{
    if (!isTLS()) return OR_Unavailable;

    // TLS mode

    return (int64_t)SSL_pending(m_ssl);
}
//=============================================================================
cerberus::OperationResult cerberus::network::Socket::sendTo(const data::ByteBuffer &buffer, const Host &dest, bool donotblock)
{
    if (isFailed()) return OR_FailedInstance;

    if (transportType() != UDP) return OR_Unavailable;

    int flags = 0;

    if (donotblock) flags |= MSG_DONTWAIT;

    Host h = dest;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(h.port);

    if (!h.hostname.empty() && !h.resolved) h.resolve();

    if (h.octet_networkOrder == 0)
        addr.sin_addr.s_addr = INADDR_ANY;
    else
        addr.sin_addr.s_addr = h.octet_networkOrder;

    auto ret = ::sendto(m_fd, buffer.data(), buffer.size(), flags, (sockaddr *)&addr, sizeof(sockaddr_in));

    if (ret == -1)
    {
        clogError("socket send error, %s", strerror(errno));
        return OR_Failure;
    }

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::network::Socket::connectP2P(const Host &dest, const time::TimeFrame &timeout)
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

    int ret = 0;

    while (timer.isRunning() || overrideFlag)
    {
        ret = ::connect(m_fd, (sockaddr *)&addr, sizeof(sockaddr_in));

        if (ret == -1)
        {
            if (errno == ECONNREFUSED)  // refused, retry
            {
                if (reopen())
                {
                    continue;
                }
            }
            else if (errno == EISCONN)  // already connected
                ret = 0;

            break;
        }
    }

    if (ret == -1)
    {
        clogError("error in socket connect: %s", strerror(errno));
        return OR_Failure;
    }

    // connect succeeded

    if (isTLS())  // TLS mode
    {
        if (m_tlsServer)
            return TLS_accept();
        else
            return TLS_connect();
    }

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::network::Socket::listen(size_t maxconn)
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
        cdebug("error in socket listen: %s", strerror(errno));
        return OR_Failure;
    }

    return OR_OK;
}
//=============================================================================
cerberus::network::Socket cerberus::network::Socket::accept(Host &peer)
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
cerberus::network::Socket cerberus::network::Socket::accept()
{
    Host peer;  // mock
    return accept(peer);
}
//=============================================================================
#ifdef LINUX_SYSTEM
cerberus::OperationResult cerberus::network::Socket::setCork(bool cork)
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
#endif
//=============================================================================
cerberus::OperationResult cerberus::network::Socket::useNagle(bool use)
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
        cdebug("error in useNagle function: %s", strerror(errno));
        return OR_Failure;  // improve error handling
    }

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::network::Socket::setTimeout(uint32_t timeout)
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

#ifdef LINUX_SYSTEM
    if (setsockopt(m_fd, IPPROTO_TCP, TCP_USER_TIMEOUT, &val, sizeof(val)) == -1)
#elif APPLE_SYSTEM
    if (setsockopt(m_fd, IPPROTO_TCP, TCP_CONNECTIONTIMEOUT, &val, sizeof(val)) == -1)
#endif
    {
        cdebug("error in setTimeout function: %s", strerror(errno));
        return OR_Failure;  // improve error handling
    }

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::network::Socket::useKeepAlive(bool use, int maxprobes, int idleTime, int interval)
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
        cdebug("error in useKeepAlive:TCP_KEEPCNT function: %s", strerror(errno));
        return OR_Failure;  // improve error handling
    }

    val = idleTime;

#ifdef LINUX_SYSTEM
    if (setsockopt(m_fd, IPPROTO_TCP, TCP_KEEPIDLE, &val, sizeof(val)) == -1)
#elif APPLE_SYSTEM
    if (setsockopt(m_fd, IPPROTO_TCP, TCP_KEEPALIVE, &val, sizeof(val)) == -1)
#endif
    {
        cdebug("error in useKeepAlive:TCP_KEEPIDLE function: %s", strerror(errno));
        return OR_Failure;  // improve error handling
    }

    val = interval;

    if (setsockopt(m_fd, IPPROTO_TCP, TCP_KEEPINTVL, &val, sizeof(val)) == -1)
    {
        cdebug("error in useKeepAlive:TCP_KEEPINTVL function: %s", strerror(errno));
        return OR_Failure;  // improve error handling
    }

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::network::Socket::useKeepAlive(bool use)
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
        cdebug("error in useKeepAlive:SO_KEEPALIVE function: %s", strerror(errno));
        return OR_Failure;  // improve error handling
    }

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::network::Socket::send(const data::filesystem::File &file)
{
    if (isFailed())
    {
        return OR_FailedInstance;
    }

    if (transportType() != TCP)
    {
        return OR_Unavailable;
    }

    if (isTLS())
    {
        auto ret = SSL_sendfile(m_ssl, file.m_fd, 0, file.size(), 0);

        if (ret < 0)
        {
            clogError("SSL socket sendfile error");
            printSSLErrors();
            return OR_Failure;
        }
        else if (ret != file.size())
        {
            clogError("SSL socket sendfile size mismatch");
            return OR_Failure;
        }

        return OR_OK;
    }

#ifdef LINUX_SYSTEM
    off_t offset  = 0;
    ssize_t bytes = 0;

    do
    {
        bytes = sendfile(m_fd, file.m_fd, &offset, file.size());
    } while (bytes > 0);

    if (bytes == -1)
    {
        logError("socket sendfile error, %s", strerror(errno));
        return OR_Failure;
    }
#elif APPLE_SYSTEM
    off_t len = 0;
    int bytes = 1;

    do
    {
        bytes = sendfile(file.m_fd, m_fd, 0, &len, nullptr, 0);
    } while (bytes > 0);

    if (bytes == -1)
    {
        clogError("socket sendfile error, %s", strerror(errno));
        return OR_Failure;
    }
#endif

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::network::Socket::recv(data::filesystem::File &file, const time::TimeFrame &timeout, const time::TimeFrame &cycTimeout)
{
    if (isFailed())
    {
        return OR_FailedInstance;
    }

    if (transportType() != TCP)
    {
        return OR_Unavailable;
    }

    data::ByteBuffer buffer;
    OperationResult ret = recv(buffer, timeout, cycTimeout);

    if (ret.fail())
    {
        return ret;
    }

    if (!file.write(buffer))
    {
        // could not write
        return OR_InvalidFile;
    }

    return OR_OK;
}
//=============================================================================
