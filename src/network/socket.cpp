#include "./socket.h"

#include <netdb.h>
#include <netinet/tcp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <poll.h>
#include <src/thread/thread.h>
#include <string.h>

#include "src/cerberus.h"
#include "src/time/timer.h"
#ifdef LINUX_SYSTEM
#include <sys/sendfile.h>
#endif
#include <unistd.h>

#include "src/data/filesystem/file.h"
#include "src/exception/exceptioncatalog.h"
#include "src/time/timer.h"

#define DEFAULT_RECV_BUFFER_SIZE 512
#define DEFAULT_MAX_CONNECTIONS 15

using namespace cerberus;

//=============================================================================
Socket::Socket(SocketType type, int fd, const Host &remote, SSL_CTX *ctx)
    : m_type(type),
      m_extern(true),
      m_maxConnections(DEFAULT_MAX_CONNECTIONS),
      m_fd(fd),
      m_streamConnected(true),
      m_recvBuffer(DEFAULT_RECV_BUFFER_SIZE),
      m_sslCtx(ctx),
      m_ssl(nullptr),
      m_bind(),
      m_remote(remote)
{
    if (ctx != nullptr)
    {
        if (SSL_CTX_up_ref(ctx) != 1)
        {
            throw cSystemExc("could not increment reference counter of SSL CTX");
        }
    }
}
//=============================================================================
void Socket::createUdpSocket()
{
    m_fd = ::socket(AF_INET, SOCK_DGRAM, 0);

    if (m_fd == -1)
    {
        logDebug("error in UDP socket creation: %s", strerror(errno));
    }
}
//=============================================================================
void Socket::createTcpSocket()
{
    m_fd = ::socket(AF_INET, SOCK_STREAM, 0);

    if (m_fd == -1)
    {
        logError("error in TCP socket creation: %s", strerror(errno));
        return;
    }

    int val = 1;
    if (::setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int)) == -1)
    {
        logError("error in setsockopt() when setting SO_REUSEADDR: %s", strerror(errno));
        close();
    }
}
//=============================================================================
void Socket::flushSocket(ByteBuffer &buffer)
{
    ByteBuffer b;

    if (isTLS())
    {
        while (TLS_pending().value != 0)
        {
            if (Socket::_recv(b, true).ok())
                buffer += b;
            else
                break;
        }

        return;
    }

    while (true)
    {
        if (Socket::_recv(b, true).ok())
            buffer += b;
        else
            break;
    }
}
//=============================================================================
OpRes Socket::_connectP2P(const Host &dest, const TimeFrame &timeout)
{
    Host h = dest;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(h.port);

    if (!h.hostname.empty() && !h.resolved) h.resolve();

    if (h.octet_networkOrder == 0)
        addr.sin_addr.s_addr = INADDR_ANY;
    else
        addr.sin_addr.s_addr = h.octet_networkOrder;

    Timer timer(timeout);

    if (!timeout.isNull()) timer.start();

    while (true)
    {
        int ret = ::connect(m_fd, (sockaddr *)&addr, sizeof(sockaddr_in));

        if (ret == 0) return OR_OK;

        if (ret == -1)
        {
            if (errno == EISCONN) return OR_OK;

            if (errno == ECONNREFUSED)
            {
                if (reset().fail()) return {OR_Failure, "socket reset fail in connect p2p"};
            }
            else
                return {OR_Failure,
                        CerberusUtils::strPrint("error in socket connectP2P: %s", strerror(errno))};
        }

        if (!timeout.isNull())
            if (!timer.isRunning()) return OR_TimedOut;
    }
}
//=============================================================================
Socket::TransportType Socket::transportType()
{
    switch (m_type)
    {
        case Socket_None:
            throw cIllegalArgExc("None type cannot be used");
        case Socket_UDP:
            return UDP;
        case Socket_TCP:
            return TCP;
        case Socket_TCPP2P:
            return TCP;
        case Socket_ICMP:
            return ICMP;
        case Socket_IPC:
            return IPC;
    }

    throw cImplMissExc("requested socket has not been implemented yet");
}
//=============================================================================
IntOpRes Socket::_accept(Host &peer)
{
    sockaddr_in addr{};
    socklen_t len = sizeof(sockaddr_in);

    int ret = ::accept(m_fd, (sockaddr *)&addr, &len);

    if (ret == -1)
    {
        return {OR_Failure, "error in socket accept", strerror(errno)};
    }

    peer.octet_networkOrder = addr.sin_addr.s_addr;
    peer.port               = ntohs(addr.sin_port);

    return ret;
}
//=============================================================================
std::string Socket::printSSLErrors()
{
    std::string ret;

    char errstr[256] = {};

    unsigned long err = 0;

    bool first = true;

    while (true)
    {
        err = ERR_get_error();

        if (err == 0) break;

        ERR_error_string_n(err, &errstr[0], sizeof(errstr));

        if (!first) ret.append(", ");
        ret.append((const char *)&errstr);

        first = false;
    }

    return ret;
}
//=============================================================================
OpRes Socket::_recv(ByteBuffer &buffer, bool donotblock)
{
    if (isFailed()) return OR_FailedInstance;

    buffer.clear();

    if (TLS_hasSession()) return _TLS_recv(buffer);

    int flags = 0;

    if (donotblock) flags |= MSG_DONTWAIT;

    sockaddr_in addr{};
    socklen_t len = sizeof(sockaddr_in);

    auto ret = ::recvfrom(m_fd, m_recvBuffer.data(), m_recvBuffer.size(), flags, (sockaddr *)&addr, &len);

    m_remote.octet_networkOrder = addr.sin_addr.s_addr;
    m_remote.port               = ntohs(addr.sin_port);

    if (ret == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return OR_WouldBlock;  // no data available at the moment
        if (errno == ENOTCONN || errno == ECONNREFUSED) m_streamConnected = false;
        return {OR_Failure, "socket recv error", strerror(errno)};
    }

    if (ret == 0)
    {
        // socket closed in case of a stream socket, zero length
        // datagram in case of a datagram socket
        m_streamConnected = false;
        return OR_Hangup;
    }

    buffer.assign(m_recvBuffer, ret);

    return OR_OK;
}
//=============================================================================
OpRes Socket::_TLS_init(const std::string &ca_file, const std::string &ca_path, const std::string &certfile,
                        const std::string &keyfile)
{
    if (isFailed()) return OR_FailedInstance;

    if (transportType() != TCP || isTLS()) return OR_Unavailable;

    if (!certfile.empty() && keyfile.empty())
    {
        return {OR_WrongArgument, "Cannot use the certificate without a private key"};
    }

    ERR_clear_error();

    m_sslCtx = SSL_CTX_new(TLS_method());

    if (m_sslCtx == NULL)
    {
        return {OR_SystemFailure, "SSL context creation failed", printSSLErrors()};
    }

    // CA file setup
    if (ca_file.empty())
    {
        if (SSL_CTX_set_default_verify_file(m_sslCtx) == 0)
        {
            _TLS_CTX_destroy();
            return {OR_Failure, "SSL set default CA file failure"};
        }
    }
    else
    {
        if (SSL_CTX_load_verify_file(m_sslCtx, ca_file.c_str()) == 0)
        {
            printSSLErrors();
            _TLS_CTX_destroy();
            return {OR_Failure, "SSL load CA file failure"};
        }
    }

    // CA path setup
    if (ca_path.empty())
    {
        if (SSL_CTX_set_default_verify_dir(m_sslCtx) == 0)
        {
            _TLS_CTX_destroy();
            return {OR_Failure, "SSL set default CA dir failure"};
        }
    }
    else
    {
        if (SSL_CTX_load_verify_dir(m_sslCtx, ca_path.c_str()) == 0)
        {
            printSSLErrors();
            _TLS_CTX_destroy();
            return {OR_Failure, "SSL load CA dir failure"};
        }
    }

    // Certificate setup
    if (!certfile.empty())
    {
        if (SSL_CTX_use_certificate_file(m_sslCtx, certfile.c_str(), SSL_FILETYPE_PEM) != 1)
        {
            printSSLErrors();
            _TLS_CTX_destroy();
            return {OR_Failure, "SSL certificate file setup failed"};
        }

        logDebug("SSL certificate file \"%s\" set", certfile.c_str());

        if (SSL_CTX_use_PrivateKey_file(m_sslCtx, keyfile.c_str(), SSL_FILETYPE_PEM) != 1)
        {
            printSSLErrors();
            _TLS_CTX_destroy();
            return {OR_Failure, "SSL private key file setup failed"};
        }

        logDebug("SSL key file \"%s\" set", keyfile.c_str());

        if (SSL_CTX_check_private_key(m_sslCtx) != 1)
        {
            printSSLErrors();
            _TLS_CTX_destroy();
            return {OR_TLSKeysCheckFail, "SSL private key check failed"};
        }
    }

    SSL_CTX_clear_mode(m_sslCtx, SSL_MODE_AUTO_RETRY);

    return OR_OK;
}
//=============================================================================
void Socket::_TLS_CTX_destroy()
{
    SSL_CTX_free(m_sslCtx);
    m_sslCtx = nullptr;
}
//=============================================================================
OpRes Socket::_TLS_send(const ByteBuffer &buffer)
{
    ERR_clear_error();

    int ret = SSL_write(m_ssl, buffer.data(), buffer.size());

    if (ret <= 0 || ret != buffer.size())
    {
        auto err = SSL_get_error(m_ssl, ret);

        OpRes r(OR_Failure, CerberusUtils::strPrint("SSL_write error %i", err));

        if (err == SSL_ERROR_SYSCALL)
        {
            if (errno == EPIPE) m_streamConnected = false;
            r.reason.append(", ");
            r.reason.append(strerror(errno));
            r = OR_SystemFailure;
        }
        else if (err == SSL_ERROR_ZERO_RETURN)
        {
            close();
            r = OR_Hangup;
        }
        else if (err == SSL_ERROR_WANT_CONNECT || err == SSL_ERROR_WANT_ACCEPT)
        {
            m_streamConnected = false;
        }
        else if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
        {
            r = OR_TemporaryUnavailable;
        }

        r.reason.append(", ");
        r.reason.append(printSSLErrors());

        return r;
    }

    if (check()) return OR_Hangup;

    return OR_OK;
}
//=============================================================================
OpRes Socket::_TLS_sendFile(const File &file)
{
    auto res = file.size();
    if (res.fail()) return res;

    auto ret = SSL_sendfile(m_ssl, file.m_fd, 0, res.value, 0);

    if (ret < 0)
    {
        auto err = SSL_get_error(m_ssl, ret);

        OpRes r(OR_Failure, CerberusUtils::strPrint("SSL_sendfile error %i", err));

        if (err == SSL_ERROR_SYSCALL)
        {
            if (errno == EPIPE) m_streamConnected = false;
            r.reason.append(", ");
            r.reason.append(strerror(errno));
            r = OR_SystemFailure;
        }
        else if (err == SSL_ERROR_ZERO_RETURN)
        {
            close();
            r = OR_Hangup;
        }
        else if (err == SSL_ERROR_WANT_CONNECT || err == SSL_ERROR_WANT_ACCEPT)
        {
            m_streamConnected = false;
        }
        else if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
        {
            r = OR_TemporaryUnavailable;
        }

        r.reason.append(", ");
        r.reason.append(printSSLErrors());

        return r;
    }
    else if (ret != res.value)
    {
        return {OR_Failure, "SSL socket sendfile size mismatch"};
    }

    if (check()) return OR_Hangup;

    return OR_OK;
}
//=============================================================================
OpRes Socket::_TLS_recv(ByteBuffer &buffer)
{
    ERR_clear_error();

    size_t readB = 0;
    int ret      = SSL_read_ex(m_ssl, m_recvBuffer.data(), m_recvBuffer.size(), &readB);

    if (ret == 0)
    {
        auto err = SSL_get_error(m_ssl, ret);

        OpRes r(OR_Failure, CerberusUtils::strPrint("SSL_read_ex error %i", err));

        if (err == SSL_ERROR_SYSCALL)
        {
            if (errno == EPIPE) m_streamConnected = false;
            r.reason.append(", ");
            r.reason.append(strerror(errno));
            r = OR_SystemFailure;
        }
        else if (err == SSL_ERROR_ZERO_RETURN)
        {
            close();
            r = OR_Hangup;
        }
        else if (err == SSL_ERROR_WANT_CONNECT || err == SSL_ERROR_WANT_ACCEPT)
        {
            m_streamConnected = false;
        }
        else if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
        {
            r = OR_TemporaryUnavailable;
        }

        r.reason.append(", ");
        r.reason.append(printSSLErrors());

        return r;
    }

    buffer.assign(m_recvBuffer, readB);

    if (check()) return OR_Hangup;

    return OR_OK;
}
//=============================================================================
OpRes Socket::_TLS_create()
{
    if (m_ssl) throw cIllegalStateExc("call to _TLS_create() twice causes memory leak");
    ERR_clear_error();
    m_ssl = SSL_new(m_sslCtx);

    if (m_ssl == NULL)
    {
        return {OR_SystemFailure, "SSL object creation failed", printSSLErrors()};
    }

    return OR_OK;
}
//=============================================================================
void Socket::_TLS_destroy()
{
    if (m_ssl)
    {
        SSL_free(m_ssl);
        m_ssl = nullptr;
    }
}
//=============================================================================
OpRes Socket::_TLS_associate()
{
    ERR_clear_error();

    if (SSL_set_fd(m_ssl, m_fd) != 1)
    {
        return {OR_Failure, "SSL association with kernel fd failed", printSSLErrors()};
    }

    return OR_OK;
}
//=============================================================================
OpRes Socket::_TLS_handshake(bool server, const Host &remote)
{
    if (!isTLS()) throw cIllegalStateExc("_TLS_handshake() called without an SSL context");

    auto res = _TLS_create();
    if (res.fail()) return res;

    res = _TLS_associate();
    if (res.fail())
    {
        _TLS_destroy();
        return res;
    }

    if (server)
        SSL_set_accept_state(m_ssl);
    else
        SSL_set_connect_state(m_ssl);

    if (remote.isTextual())
        if (SSL_set_tlsext_host_name(m_ssl, remote.hostname.c_str()) == 0)
        {
            _TLS_destroy();
            return {OR_Failure, "SSL_set_tlsext_host_name failed"};
        }

    if (SSL_do_handshake(m_ssl) != 1)
    {
        _TLS_destroy();
        return {OR_Failure, "SSL_do_handshake failed", printSSLErrors()};
    }

    return OR_OK;
}
//=============================================================================
OpRes Socket::_TLS_shutdown(bool quick)
{
    if (!TLS_hasSession()) return OR_Unavailable;

    ERR_clear_error();

    do
    {
        int ret = SSL_shutdown(m_ssl);

        if (ret == 1)  // bidirectional shutdown completed
        {
            break;
        }
        else if (ret < 0)  // error
        {
            return {OR_Failure, "SSL socket shutdown error", printSSLErrors()};
        }

    } while (!quick);

    _TLS_destroy();

    return OR_OK;
}
//=============================================================================
Socket::Socket(SocketType type)
    : m_type(type),
      m_extern(false),
      m_maxConnections(DEFAULT_MAX_CONNECTIONS),
      m_fd(-1),
      m_streamConnected(false),
      m_recvBuffer(DEFAULT_RECV_BUFFER_SIZE),
      m_sslCtx(nullptr),
      m_ssl(nullptr),
      m_bind()
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
            throw cImplMissExc("ICMP sockets not implemented yet");
            break;
        case IPC:
            throw cImplMissExc("IPC sockets not implemented yet");
            break;
    }
}
//=============================================================================
Socket::~Socket()
{
    close();
    TLS_deinit();
}
//=============================================================================
OpRes Socket::bind(const Host &iface)
{
    if (isFailed()) return OR_FailedInstance;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(iface.port);

    addr.sin_addr.s_addr = iface.octet_networkOrder;
    int ret              = ::bind(m_fd, (sockaddr *)&addr, sizeof(sockaddr_in));

    if (ret == -1)
    {
        return {OR_Failure, CerberusUtils::strPrint("error in socket bind: %s", strerror(errno))};
    }

    m_bind = iface;  // backup iface

    return OR_OK;
}
//=============================================================================
OpRes Socket::connect(const Host &dest)
{
    if (isFailed()) return OR_FailedInstance;
    if (m_streamConnected) return OR_Unavailable;

    Host h = dest;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(h.port);

    if (!h.hostname.empty() && !h.resolved) h.resolve();

    if (h.octet_networkOrder == 0)
        addr.sin_addr.s_addr = INADDR_ANY;
    else
        addr.sin_addr.s_addr = h.octet_networkOrder;

    int ret = ::connect(m_fd, (sockaddr *)&addr, sizeof(sockaddr_in));

    if (ret == -1)
    {
        if (transportType() == TCP) close();  // the socket is not usable anymore
        return {OR_Failure, CerberusUtils::strPrint("error in socket connect: %s", strerror(errno))};
    }

    if (transportType() == TCP) m_streamConnected = true;

    if (!isTLS()) return OR_OK;

    _TLS_destroy();

    if (_TLS_handshake(false, dest).fail()) return OR_Failure;

    return OR_OK;
}
//=============================================================================
OpRes Socket::send(const ByteBuffer &buffer, bool donotblock)
{
    if (isFailed()) return OR_FailedInstance;

    if (TLS_hasSession()) return _TLS_send(buffer);

    int flags = 0;

    if (donotblock) flags |= MSG_DONTWAIT;

    auto ret = ::send(m_fd, buffer.data(), buffer.size(), flags);

    if (ret == -1)
    {
        if (errno == ENOTCONN || errno == ECONNRESET || errno == EPIPE) m_streamConnected = false;
        return {OR_Failure, CerberusUtils::strPrint("socket send error, %s", strerror(errno))};
    }

    return OR_OK;
}
//=============================================================================
OpRes Socket::recv(ByteBuffer &buffer, const TimeFrame &timeout, const TimeFrame &cycTimeout)
{
    if (isFailed()) return OR_FailedInstance;

    if (timeout.isNull()) return _recv(buffer, true);

    ByteBuffer b;
    buffer.clear();
    bool first = true;

    while (true)
    {
        auto res = waitRead((first || cycTimeout.isNull()) ? timeout : cycTimeout);

        switch (res.res)
        {
            case OR_OK:
                break;  // continue execution

            case OR_TimedOut:
                if (TLS_hasSession()) flushSocket(buffer);
                if (buffer.size() == 0) return OR_TimedOut;
                return OR_OK;

            case OR_Hangup:
                flushSocket(buffer);
                return {OR_Hangup, "poll returned hangup"};

            default:
                return res;
        };

        res = Socket::_recv(b, false);

        switch (res.res)
        {
            case OR_OK:
                first = false;
                buffer += b;
                break;

            case OR_Hangup:
                if (TLS_hasSession()) flushSocket(buffer);
                return {OR_Hangup, "recv returned hangup"};

            case OR_TemporaryUnavailable:
                continue;

            default:
                return res;
        };
    }
}
//=============================================================================
OpRes Socket::recv(ByteBuffer &buffer) { return _recv(buffer, false); }
//=============================================================================
void Socket::setMaxConnections(size_t maxconn) { m_maxConnections = maxconn; }
//=============================================================================
bool Socket::isFailed() const { return (m_fd == -1) || (m_type == Socket_None); }
//=============================================================================
bool Socket::isConnected() const
{
    if (isTLS())
    {
        if (!TLS_hasSession()) return false;
        if (!m_streamConnected) return false;
        if (TLS_getshutdown().value.any()) return false;

        return true;
    }

    return m_streamConnected;
}
//=============================================================================
void Socket::setRecvBufferSize(size_t size) { m_recvBuffer.resize(size); }
//=============================================================================
const Host &Socket::remote() { return m_remote; }
//=============================================================================
OpRes Socket::canRead()
{
    if (isFailed()) return OR_FailedInstance;

    pollfd set{};
    set.fd     = m_fd;
    set.events = POLLIN;

    int ret = poll(&set, 1, 0);

    if (ret == 0) return OR_TimedOut;  // timeout

    if (ret == -1)
    {
        return {OR_SystemFailure, CerberusUtils::strPrint("error in poll: %s", strerror(errno))};
    }

    if (set.revents & POLLIN) return OR_OK;

    if (set.revents & POLLERR) return OR_Failure;

    if (set.revents & POLLHUP) return OR_Hangup;

    if (set.revents & POLLNVAL) return OR_BadConditions;

    return OR_Failure;
}
//=============================================================================
OpRes Socket::waitRead(const TimeFrame &timeout)
{
    if (isFailed()) return OR_FailedInstance;

    pollfd set{};
    set.fd     = m_fd;
    set.events = POLLIN;

    int ret = poll(&set, 1, timeout.isNull() ? -1 : timeout.toMilliseconds());

    if (ret == 0) return OR_TimedOut;  // timeout

    if (ret == -1)
    {
        return {OR_SystemFailure, CerberusUtils::strPrint("error in poll: %s", strerror(errno))};
    }

    if (set.revents & POLLIN) return OR_OK;

    if (set.revents & POLLERR) return OR_Failure;

    if (set.revents & POLLHUP) return OR_Hangup;

    if (set.revents & POLLNVAL) return OR_BadConditions;

    return OR_Failure;
}
//=============================================================================
OpRes Socket::waitWrite(const TimeFrame &timeout)
{
    if (isFailed()) return OR_FailedInstance;

    pollfd set{};
    set.fd     = m_fd;
    set.events = POLLOUT;

    int ret = poll(&set, 1, timeout.isNull() ? -1 : timeout.toMilliseconds());

    if (ret == 0) return OR_TimedOut;  // timeout

    if (ret == -1)
    {
        return {OR_SystemFailure, CerberusUtils::strPrint("error in poll: %s", strerror(errno))};
    }

    if (set.revents & POLLOUT) return OR_OK;

    if (set.revents & POLLERR) return OR_Failure;

    if (set.revents & POLLHUP) return OR_Hangup;

    if (set.revents & POLLNVAL) return OR_BadConditions;

    return OR_Failure;
}
//=============================================================================
OpRes Socket::close()
{
    if (isFailed()) return OR_FailedInstance;

    if (TLS_hasSession())
    {
        TLS_shutdown(true);  // send close_notify alert to the peer
    }

    ::shutdown(m_fd, SHUT_WR);  // discard return value

    OpRes res(OR_OK);

    if (::close(m_fd) == -1)
    {
        res = OpRes(OR_Failure, CerberusUtils::strPrint("socket close error, %s", strerror(errno)));
    }

    m_fd              = -1;
    m_streamConnected = false;

    return res;
}
//=============================================================================
bool Socket::check()
{
    if (isConnected()) return false;

    close();
    return true;
}
//=============================================================================
OpRes Socket::reset()
{
    ::close(m_fd);
    m_fd              = -1;
    m_streamConnected = false;

    createTcpSocket();  // recreate the socket

    if (m_fd == -1) return OR_Failure;

    if (m_bind.isValid())
        if (bind(m_bind).fail()) return OR_Failure;

    if (TLS_hasSession())
    {
        if (_TLS_associate().fail()) return OR_Failure;

        if (SSL_clear(m_ssl) != 1)  // reset the TLS status
        {
            return {OR_Failure, "SSL_clear failed", printSSLErrors()};
        }
    }

    return OR_OK;
}
//=============================================================================
bool Socket::isTLS() const { return m_sslCtx; }
//=============================================================================
OpRes Socket::TLS_init(const std::string &certfile, const std::string &keyfile, const std::string &ca_file,
                       const std::string &ca_path)
{
    return _TLS_init(ca_file, ca_path, certfile, keyfile);
}
//=============================================================================
OpRes Socket::TLS_deinit()
{
    if (!isTLS()) return OR_Unavailable;

    if (TLS_hasSession())
    {
        TLS_shutdown();  // send close_notify alert to the peer
    }

    _TLS_CTX_destroy();  // decrement reference counter
    m_sslCtx = nullptr;

    return OR_OK;
}
//=============================================================================
OpRes Socket::TLS_handshake(bool server, const Host &peer)
{
    if (isFailed()) return OR_FailedInstance;
    if (!isTLS() || TLS_hasSession()) return OR_Unavailable;

    OpRes res = _TLS_handshake(server, peer);

    if (res.fail()) return res;

    return OR_OK;
}
//=============================================================================
bool Socket::TLS_hasSession() const { return m_ssl; }
//=============================================================================
OpRes Socket::TLS_shutdown(bool quick) { return _TLS_shutdown(quick); }
//=============================================================================
OpResData<TLS_SD_STATE> Socket::TLS_getshutdown() const
{
    if (!TLS_hasSession()) return OR_Unavailable;

    int ret = SSL_get_shutdown(m_ssl);

    bool local  = false;
    bool remote = false;

    if (ret & SSL_SENT_SHUTDOWN) local = true;

    if (ret & SSL_RECEIVED_SHUTDOWN) remote = true;

    return TLS_SD_STATE(local, remote);
}
//=============================================================================
OpRes Socket::TLS_ignoreHangup(bool ignore)
{
    if (!isTLS()) return OR_Unavailable;

    if (ignore)
    {
        SSL_CTX_set_options(m_sslCtx, SSL_OP_IGNORE_UNEXPECTED_EOF);

        if (TLS_hasSession())
        {
            SSL_set_options(m_ssl, SSL_OP_IGNORE_UNEXPECTED_EOF);
        }
    }
    else
    {
        SSL_CTX_clear_options(m_sslCtx, SSL_OP_IGNORE_UNEXPECTED_EOF);

        if (TLS_hasSession())
        {
            SSL_clear_options(m_ssl, SSL_OP_IGNORE_UNEXPECTED_EOF);
        }
    }

    return OR_OK;
}
//=============================================================================
StringOpRes Socket::TLS_getProtocolName()
{
    if (!TLS_hasSession()) return OR_Unavailable;

    return std::string(SSL_get_cipher_version(m_ssl));
}
//=============================================================================
StringOpRes Socket::TLS_getCipherName()
{
    if (!TLS_hasSession()) return OR_Unavailable;

    return std::string(SSL_get_cipher_name(m_ssl));
}
//=============================================================================
BoolOpRes Socket::TLS_securePeerRenegSupport()
{
    if (!TLS_hasSession()) return OR_Unavailable;

    if (SSL_get_secure_renegotiation_support(m_ssl) == 1) return true;

    return false;
}
//=============================================================================
BoolOpRes Socket::TLS_hasPending()
{
    if (!TLS_hasSession()) return OR_Unavailable;

    if (SSL_has_pending(m_ssl) == 1) return true;

    return false;
}
//=============================================================================
IntOpRes Socket::TLS_pending()
{
    if (!TLS_hasSession()) return OR_Unavailable;

    return (int64_t)SSL_pending(m_ssl);
}
//=============================================================================
OpRes Socket::sendTo(const ByteBuffer &buffer, const Host &dest, bool donotblock)
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
        return {OR_Failure, CerberusUtils::strPrint("socket send error, %s", strerror(errno))};
    }

    return OR_OK;
}
//=============================================================================
OpRes Socket::connectP2P(const Host &dest, const TimeFrame &timeout)
{
    if (isFailed()) return OR_FailedInstance;
    if (m_type != Socket_TCPP2P || m_streamConnected) return OR_Unavailable;

    auto res = _connectP2P(dest, timeout);

    if (res.ok()) m_streamConnected = true;

    return res;
}
//=============================================================================
OpRes Socket::listen(size_t maxconn)
{
    if (isFailed()) return OR_FailedInstance;

    if (m_extern || transportType() != TCP) return OR_Unavailable;

    if (::listen(m_fd, maxconn == 0 ? m_maxConnections : maxconn) == -1)
    {
        return {OR_Failure, CerberusUtils::strPrint("error in socket listen: %s", strerror(errno))};
    }

    return OR_OK;
}
//=============================================================================
Socket *Socket::accept()
{
    if (isFailed() || transportType() != TCP || m_extern) return nullptr;

    Host h;
    auto fd = _accept(h);

    if (fd.fail()) return nullptr;

    return new Socket(m_type, static_cast<int>(fd.value), h, m_sslCtx);
}
//=============================================================================
#ifdef LINUX_SYSTEM
OpRes Socket::setCork(bool cork)
{
    if (isFailed()) return OR_FailedInstance;

    if (transportType() != TCP) return OR_Unavailable;

    int val = cork ? 1 : 0;

    if (setsockopt(m_fd, IPPROTO_TCP, TCP_CORK, &val, sizeof(val)) == -1)
    {
        return {OR_Failure, CerberusUtils::strPrint("error in setCork function: %s", strerror(errno))};
    }

    return OR_OK;
}
#endif
//=============================================================================
OpRes Socket::useNagle(bool use)
{
    if (isFailed()) return OR_FailedInstance;

    if (transportType() != TCP) return OR_Unavailable;

    int val = use ? 1 : 0;

    if (setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val)) == -1)
    {
        return {OR_Failure, CerberusUtils::strPrint("error in useNagle function: %s", strerror(errno))};
    }

    return OR_OK;
}
//=============================================================================
OpRes Socket::setTimeout(uint32_t timeout)
{
    if (isFailed()) return OR_FailedInstance;

    if (transportType() != TCP) return OR_Unavailable;

    unsigned int val = timeout;

#ifdef LINUX_SYSTEM
    if (setsockopt(m_fd, IPPROTO_TCP, TCP_USER_TIMEOUT, &val, sizeof(val)) == -1)
#elif APPLE_SYSTEM
    if (setsockopt(m_fd, IPPROTO_TCP, TCP_CONNECTIONTIMEOUT, &val, sizeof(val)) == -1)
#endif
    {
        return {OR_Failure, CerberusUtils::strPrint("error in setTimeout function: %s", strerror(errno))};
    }

    return OR_OK;
}
//=============================================================================
OpRes Socket::useKeepAlive(bool use, int maxprobes, int idleTime, int interval)
{
    if (isFailed()) return OR_FailedInstance;

    if (transportType() != TCP) return OR_Unavailable;

    auto ret = useKeepAlive(use);

    if (ret != OR_OK) return ret;

    if (!use) return OR_OK;

    int val = maxprobes;

    if (setsockopt(m_fd, IPPROTO_TCP, TCP_KEEPCNT, &val, sizeof(val)) == -1)
    {
        return {OR_Failure,
                CerberusUtils::strPrint("error in useKeepAlive:TCP_KEEPCNT function: %s", strerror(errno))};
    }

    val = idleTime;

#ifdef LINUX_SYSTEM
    if (setsockopt(m_fd, IPPROTO_TCP, TCP_KEEPIDLE, &val, sizeof(val)) == -1)
#elif APPLE_SYSTEM
    if (setsockopt(m_fd, IPPROTO_TCP, TCP_KEEPALIVE, &val, sizeof(val)) == -1)
#endif
    {
        return {OR_Failure,
                CerberusUtils::strPrint("error in useKeepAlive:TCP_KEEPIDLE function: %s", strerror(errno))};
    }

    val = interval;

    if (setsockopt(m_fd, IPPROTO_TCP, TCP_KEEPINTVL, &val, sizeof(val)) == -1)
    {
        return {OR_Failure,
                CerberusUtils::strPrint("error in useKeepAlive:TCP_KEEPINTVL function: %s", strerror(errno))};
    }

    return OR_OK;
}
//=============================================================================
OpRes Socket::useKeepAlive(bool use)
{
    if (isFailed()) return OR_FailedInstance;

    if (transportType() != TCP) return OR_Unavailable;

    int val = use ? 1 : 0;

    if (setsockopt(m_fd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val)) == -1)
    {
        return {OR_Failure,
                CerberusUtils::strPrint("error in useKeepAlive:SO_KEEPALIVE function: %s", strerror(errno))};
    }

    return OR_OK;
}
//=============================================================================
OpRes Socket::send(const File &file)
{
    if (isFailed()) return OR_FailedInstance;

    if (transportType() != TCP) return OR_Unavailable;

    if (TLS_hasSession()) return _TLS_sendFile(file);

    auto res = file.size();
    if (res.fail()) return res;

#ifdef LINUX_SYSTEM
    off_t offset  = 0;
    ssize_t bytes = 0;

    do
    {
        bytes = sendfile(m_fd, file.m_fd, &offset, res.value);
    } while (bytes > 0);

    if (bytes == -1)
    {
        return {OR_Failure, CerberusUtils::strPrint("socket sendfile error, %s", strerror(errno))};
    }
#elif APPLE_SYSTEM
    off_t len = 0;  // send until EOF
    int bytes = 1;

    do
    {
        bytes = sendfile(file.m_fd, m_fd, 0, &len, nullptr, 0);
    } while (bytes > 0);

    if (bytes == -1)
    {
        return {OR_Failure, CerberusUtils::strPrint("socket sendfile error, %s", strerror(errno))};
    }
#endif

    return OR_OK;
}
//=============================================================================
OpRes Socket::recv(File &file, const TimeFrame &timeout, const TimeFrame &cycTimeout)
{
    if (isFailed()) return OR_FailedInstance;

    if (transportType() != TCP) return OR_Unavailable;

    ByteBuffer buffer;
    OpRes ret = recv(buffer, timeout, cycTimeout);

    if (!buffer.isEmpty())
    {
        if (file.write(buffer).fail()) return OR_InvalidFile;
    }

    return ret;
}
//=============================================================================
