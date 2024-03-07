#ifndef CERBERUS_NETWORK_SOCKET_H
#define CERBERUS_NETWORK_SOCKET_H

#include "../core/cerberusobject.h"
#include "../data/bytebuffer.h"
#include "../time/timeframe.h"
#include "../types.h"

#define UDPSocket(namestr) cerberus::network::Socket(cerberus::core::CerberusObject::Socket_UDP, namestr)
#define TCPSocket(namestr) cerberus::network::Socket(cerberus::core::CerberusObject::Socket_TCP, namestr)
#define TCPP2PSocket(namestr) \
    cerberus::network::Socket(cerberus::core::CerberusObject::Socket_TCPP2P, namestr)

// A socket capable of great things!

// This is the Cerberus Socket class. It defines a socket usable in all circumstances.
// This class implements stream socket as well as datagram, HTTP, FTP, ICMP sockets and so on.
// The socket type must be specified in the constructor

typedef struct ssl_ctx_st SSL_CTX;
typedef struct ssl_st SSL;

namespace cerberus
{
    namespace data
    {
        namespace filesystem
        {
            class File;
        }
    }  // namespace data

    namespace network
    {
        class Socket : public core::CerberusObject
        {
           private:
            Socket(SocketType type, int fd, SSL_CTX* ctx = nullptr);

            // This method creates a datagram socket and assigns the resulting file descriptor to
            // m_fd. Check is the socket isFailed() after this call
            void createUdpSocket();

            // This method creates a stream socket and assigns the resulting file descriptor to
            // m_fd. Check is the socket isFailed() after this call
            void createTcpSocket();

            // Receive until EOF
            void flushSocket(data::ByteBuffer& buffer);

            enum TransportType
            {
                TCP,
                UDP,
                ICMP,
                IPC,
            };

            TransportType transportType();

            int _accept(Host& peer);

            void printSSLErrors();

            OpRes _recv(data::ByteBuffer& buffer, bool donotblock);

            OpRes _TLS_init(const std::string& ca_file, const std::string& ca_path,
                            const std::string& certfile, const std::string& keyfile);

            OpRes _TLS_send(const data::ByteBuffer& buffer);

            OpRes _TLS_sendFile(const data::filesystem::File& file);

            OpRes _TLS_recv(data::ByteBuffer& buffer);

            OpRes _TLS_create();

            void _TLS_destroy();

            OpRes _TLS_associate();

            OpRes _TLS_handshake(bool server, const Host& remote = Host());

            OpRes _TLS_shutdown(bool quick);

            bool m_extern;  // used for acceptable sockets

            size_t m_maxConnections;  // used for acceptable sockets

            int m_fd;

            bool m_streamConnected;

            data::ByteBuffer m_recvBuffer;

            SSL_CTX* m_sslCtx;  // for ssl
            SSL* m_ssl;

            Host m_bind;

           public:
            Socket() = delete;

            Socket(const Socket& other) = delete;

            Socket(SocketType type, const std::string& name = std::string());

            virtual ~Socket();

            // GENERAL SECTION:

            // Bind this socket to a given interface
            OpRes bind(const Host& iface);

            // Connect this socket to a remote Host. If hostname field is not empty and
            // not yet resolved, this method will call Host::resolve() internally.
            // If the socket is configured as a TLS socket (i.e. the TLS_init() has been called)
            // then this method will also do a client TLS handshake.
            OpRes connect(const Host& dest);

            // Send out a buffer
            OpRes send(const data::ByteBuffer& buffer, bool donotblock = false);

            // Wait for data to arrive then receive and store them in the buffer.
            // The returned amount of data may be greater than the internal buffer size, since this
            // method actually calls the system recv() more times until no more data are present,
            // and merges the buffers together.
            // - If timeout is invalid, the call acts as a non-blocking call and will return data,
            // or OR_WouldBlock if the operation would block.
            // - If timeout is a valid timeout, the call waits for data for at most timeout time,
            // then starts receiving data,
            //   and then waits for new data become available or for timeout again. If the timeout
            //   is reached, and the call was able to receive some data, this method will return
            //   OR_OK. If timeout is reached but the call was not able to get any data, it returns
            //   OR_TimedOut.
            // - If cycTimeout is valid, the call uses timeout for the first recv(), and cycTimeout
            // for the subsequent recv() cyclic calls
            OpRes recv(data::ByteBuffer& buffer, const time::TimeFrame& timeout,
                       const time::TimeFrame& cycTimeout = time::TimeFrame());

            // Receive a buffer. This call will always block if necessary
            // The returned amount of data will not exceed the internal buffer size
            OpRes recv(data::ByteBuffer& buffer);

            // Set the maximum incoming connections number. If the queue is full, new peers
            // will receive a connection refused error when they do a connect().
            // This method is designed for TCP based sockets, however, calling this method on a
            // non-TCP socket won't have any effect
            void setMaxConnections(size_t maxconn);

            // Check if the socket is a failed socket
            bool isFailed() const;

            // Check if the socket connection is established.
            // If the socket is a TLS socket, this method behaves exactly as TLS_hasSession(),
            // otherwise, it will tell if the transport layer is present (TCP)
            bool isConnected() const;

            // Set the buffer size used for recv calls, default is 512 bytes
            void setRecvBufferSize(size_t size);

            // Block until read operation is available.
            // Passing an invalid time will make the call block forever
            OpRes waitRead(const time::TimeFrame& timeout = time::TimeFrame());

            // Block until write operation is available
            // Passing an invalid time will make the call block forever
            OpRes waitWrite(const time::TimeFrame& timeout = time::TimeFrame());

            // Close the socket
            // This method must be called at most one time,
            // the subsequent calls will return OR_FailedInstance.
            // This method closes the system socket but it does not de-initialize the TLS layer,
            // so it can be reused later after a reset() call.
            // If the TLS layer is present, this method will send the close_notify alert to the peer
            // before closing the socket.
            OpRes close();

            // Close the socket and create a new one bound to the same interface if any.
            // The SSL layer is kept and re-associated to the new socket file descriptor.
            // NOTE: the SSL status is cleared but not totally reset,
            // see https://www.openssl.org/docs/manmaster/man3/SSL_clear.html
            // To get a total TLS reset, call TLS_reset() after this call.
            OpRes reset();

            // TLS-enabled sockets (stream):

            // Check if the socket has an initted TLS context
            bool isTLS() const;

            // Initialize the TLS context. The socket is setup to negotiate the highest version
            // possible with the peer.
            // Future sockets obtained with the accept() on this instance will inherit its context
            // (certificates and keys).
            //
            // -The certfile parameter specifies the certificate file to use.
            //
            // -The keyfile parameter specifies the private key file associated with the
            //  given certificate. If a certificate has been provided, this string cannot be empty.
            //
            // -The ca_file parameter specifies the .pem file that contains one or more
            //  certificates. If it is an empty string, the default path is used.
            //
            // -The ca_path parameter specifies the directory that contains several single
            //  certificates. If it is an empty string, the default path is used.
            OpRes TLS_init(const std::string& certfile = "", const std::string& keyfile = "",
                           const std::string& ca_file = "", const std::string& ca_path = "");

            // Free all the allocated resources for the TLS features, thus, a call to initTLS() is
            // necessary for the socket to send and receive on the secure layer again.
            // This method also shuts down an active TLS session if present.
            OpRes TLS_deinit();

            // Perform the TLS handshake with the other peer.
            // The server parameter specifies if the socket has to be configured as client or
            // server socket.
            OpRes TLS_handshake(bool server = false, const Host& peer = Host());

            // Tell if the socket is currently connected to a remote peer
            bool TLS_hasSession() const;

            // Shutdown the TLS connection.
            // If quick is false, the method will block until the whole shutdown
            // process is completed. This prevents truncation attacks.
            // Otherwise, if quick is true, the method will just send the close-notify alert
            // to the other peer and consider the TLS layer closed
            OpRes TLS_shutdown(bool quick = false);

            // Set the Socket to ignore the Hangup signal from the peer before a shutdown is
            // completed. After this call, the Socket will not give an error inside a recv()
            // operation if the peer closes the connection suddently without doing a shutdown
            // before. The ignore parameter specifies the status of the option.
            OpRes TLS_ignoreHangup(bool ignore = true);

            // Get the protocol name chosen after TLS handshake
            StringOpRes TLS_getProtocolName();

            // Get the cipher name chosen after TLS handshake
            StringOpRes TLS_getCipherName();

            // Get the renegotiation support from the peer as bool
            BoolOpRes TLS_securePeerRenegSupport();

            // Tell if there are data in the buffer (processed or not)
            BoolOpRes TLS_hasPending();

            // Tell the number of bytes available to read
            IntOpRes TLS_pending();

            // UDP SECTION:

            OpRes sendTo(const data::ByteBuffer& buffer, const Host& dest, bool donotblock = false);

            // TCP P2P SECTION:

            // Connect this socket to a remote Host. If hostname field is not empty and
            // not yet resolved, this method will call Host::resolve() internally.
            // If the timeout argument is invalid, the method will try to connect for ever.
            // If the Socket is a TLS socket, to reach the complete connected state, the application
            // must afterwards call TLS_handshake() and specify the mode.
            OpRes connectP2P(const Host& dest, const time::TimeFrame& timeout = time::TimeFrame());

            // TCP (or derived) SECTION:

            // Mark the socket as a listening socket, so it can accept() new connections.
            // If the instance of the socket was returned by an accept or the socket transport is
            // not TCP, this method will return SO_Unavailable
            OpRes listen(size_t maxconn = 0);

            // Block until a new connection is available and return the new socket.
            // Return the requesting peer as an Host object
            // If the instance of the socket was returned by an accept or the socket transport is
            // not TCP, this method will return an invalid socket. If this instance is configured
            // with TLS, the returned Socket will inherit the basic init settings used in the
            // TLS_init() call previously
            Socket accept(Host& peer);

            // Block until a new connection is available and return the new socket
            // If the instance of the socket was returned by an accept or the socket transport is
            // not TCP, this method will return an invalid socket. If this instance is configured
            // with TLS, the returned Socket will inherit the basic init settings used in the
            // TLS_init() call previously
            Socket accept();

#ifdef LINUX_SYSTEM
            // If cork == true then stop sending out frames with send()
            // If cork == false then send all the outgoing queue
            // This is useful to compose a buffer using multiple send() calls and then
            // send all the buffer at once calling setCork(false)
            OperationResult setCork(bool cork = true);
#endif
            // Set the Nagle algorithm state. As default, TCP sockets have it enabled
            OpRes useNagle(bool use = true);

            // Set the connection timeout of the socket. Default is 20 minutes (system dependent)
            // A value of zero corresponds to the system default
            OpRes setTimeout(uint32_t timeout);

            // Enable keep alive probes usage.
            // When the connection goes idle and <idleTime> passed, TCP will start sending
            // keep alive probes every <interval> seconds to know if the counterpart is still
            // connected. TCP will drop the connection when <maxprobes> keep alive messages did not
            // receive a response. maxprobes is the maximum number of failed probes, idleTime and
            // interval are expressed in seconds
            OpRes useKeepAlive(bool use, int maxprobes, int idleTime, int interval);

            // Like the method above but leaving parameters unchanged
            OpRes useKeepAlive(bool use = true);

            // Send out a file. This method is optimized and the buffer copy procedure
            // happens in kernel space, so it's really fast and efficient.
            // The file has to be open before this call
            OpRes send(const data::filesystem::File& file);

            // Receive a file.
            // This method blocks until a file is received or timeout is reached.
            // -If timeout is not specified and no data are ready to be received, this call could
            // block for ever -If timeout (or/and cycTimeout) is specified, the behavior is the same
            // as the recv() call
            OpRes recv(data::filesystem::File& file, const time::TimeFrame& timeout = time::TimeFrame(),
                       const time::TimeFrame& cycTimeout = time::TimeFrame());
        };
    }  // namespace network
}  // namespace cerberus

#endif  // CERBERUS_NETWORK_SOCKET_H
