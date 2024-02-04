#ifndef CERBERUS_NETWORK_SOCKET_H
#define CERBERUS_NETWORK_SOCKET_H

#include "../core/cerberusobject.h"
#include "../data/bytebuffer.h"
#include "../time/timeframe.h"
#include "../types.h"

#define UDPSocket(namestr) cerberus::network::Socket(cerberus::core::CerberusObject::Socket_UDP, namestr)
#define TCPSocket(namestr) cerberus::network::Socket(cerberus::core::CerberusObject::Socket_TCP, namestr)
#define TCPP2PSocket(namestr) cerberus::network::Socket(cerberus::core::CerberusObject::Socket_TCPP2P, namestr)

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

            // This method creates a datagram socket and assigns the resulting file descriptor to m_fd.
            // Check is the socket isFailed() after this call
            void createUdpSocket();

            // This method creates a stream socket and assigns the resulting file descriptor to m_fd.
            // Check is the socket isFailed() after this call
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

            OperationResult _recv(data::ByteBuffer& buffer, bool donotblock);

            OperationResult TLS_connect();

            OperationResult TLS_accept();

            OperationResult TLS_send(const data::ByteBuffer& buffer);

            OperationResult TLS_sendFile(const data::filesystem::File& file);

            OperationResult TLS_recv(data::ByteBuffer& buffer);

            bool m_extern;  // used for acceptable sockets

            size_t m_maxConnections;  // used for acceptable sockets

            int m_fd;

            bool m_forceTLSServer;

            bool m_connected;

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
            OperationResult bind(const Host& iface);

            // Connect this socket to a remote Host. If hostname field is not empty and
            // not yet resolved, this method will call Host::resolve() internally
            OperationResult connect(const Host& dest);

            // Send out a buffer
            OperationResult send(const data::ByteBuffer& buffer, bool donotblock = false);

            // Wait for data to arrive then receive and store them in the buffer.
            // The returned amount of data may be greater than the internal buffer size, since this method
            // actually calls the system recv() more times until no more data are present, and merges the buffers together.
            // - If timeout is invalid, the call acts as a non-blocking call and will return data, or OR_WouldBlock if the operation would block.
            // - If timeout is a valid timeout, the call waits for data for at most timeout time, then starts receiving data,
            //   and then waits for new data become available or for timeout again. If the timeout is reached, and the call was able to receive
            //   some data, this method will return OR_OK. If timeout is reached but the call was not able to get any data, it returns OR_TimedOut.
            // - If cycTimeout is valid, the call uses timeout for the first recv(), and cycTimeout for the subsequent recv() cyclic calls
            OperationResult recv(data::ByteBuffer& buffer, const time::TimeFrame& timeout, const time::TimeFrame& cycTimeout = time::TimeFrame());

            // Receive a buffer. This call will always block if necessary
            // The returned amount of data will not exceed the internal buffer size
            OperationResult recv(data::ByteBuffer& buffer);

            // Set the maximum incoming connections number. If the queue is full, new peers
            // will receive a connection refused error when they do a connect().
            // This method is designed for TCP based sockets, however, calling this method on a non-TCP
            // socket won't have any effect
            void setMaxConnections(size_t maxconn);

            // Check if the socket is a failed socket
            bool isFailed() const;

            // Check if the socket connection is established
            bool isConnected() const;

            // Set the buffer size used for recv calls, default is 512 bytes
            void setRecvBufferSize(size_t size);

            // Block until read operation is available.
            // Passing an invalid time will make the call block forever
            OperationResult waitRead(const time::TimeFrame& timeout = time::TimeFrame());

            // Block until write operation is available
            // Passing an invalid time will make the call block forever
            OperationResult waitWrite(const time::TimeFrame& timeout = time::TimeFrame());

            // Close the socket
            // This method must be called at most one time,
            // the subsequent calls will return OR_FailedInstance
            OperationResult close();

            // Close the socket and create a new one bound to the same interface if any.
            // The SSL layer status will be lost, and eventually must be restored with SSL_Init()
            OperationResult reset();

            // TLS-enabled sockets (stream):

            // Get the TLS flag
            // false = The instance is not configured as a TLS Socket
            // true = The instance is configured as a TLS Socket
            bool isTLS();

            // Set the socket to be a TLS socket. The socket will negotiate the highest version possible
            // with the peer. This method creates a new SSL context and a new SSL object.
            // Future sockets obtained with the accept() on this instance will inherit its context (cert file and key file).
            // If forceServer is set, the Socket will be configured as a TLS server socket. Otherwise, the handshake will decide the type.
            // Also, the state of forceServer will be crucial for connectP2P() call in TLS mode (see below).
            OperationResult TLS_init(const std::string& certfile = "", const std::string& keyfile = "", bool forceServer = false);

            // Free all the allocated resources for the TLS features, thus, a call to initTLS() is necessary
            // for the socket to send and receive on the secure layer again.
            OperationResult TLS_deinit();

            // Shutdown the TLS connection.
            // In case of success, the bool value contained in the returned
            // result tells the status of the shutdown (false = unidirectional, true = bidirectional)
            OperationResult TLS_shutdown();

            // Set the Socket to ignore the Hangup signal from the peer before a shutdown is completed.
            // After this call, the Socket will not give an error inside a recv() operation if the peer closes the
            // connection suddently without doing a shutdown before.
            // If the restrict parameter is true, this setting will be applied at this instance only, otherwise, the setting
            // will be applied also at the TLS context, modifying also every socket generated by accept() called from this instance.
            OperationResult TLS_ignoreHangup(bool restrict = false);

            // Obtain the shutdown status of the TLS layer. The returned values is contained in the int member of result
            // 0 : no shutdown sent or received
            // 1 : shutdown sent
            // 2 : shutdown received
            // 3 : shutdown sent and received
            OperationResult TLS_getShutdown();

            // Get the protocol name chosen after TLS handshake
            std::string TLS_getProtocolName();

            // Get the cipher name chosen after TLS handshake
            std::string TLS_getCipherName();

            // Get the renegotiation support from the peer as bool
            OperationResult TLS_securePeerRenegSupport();

            // Tell if there are data in the buffer (processed or not)
            OperationResult TLS_hasPending();

            // Tell the number of bytes available to read
            OperationResult TLS_pending();

            // UDP SECTION:

            OperationResult sendTo(const data::ByteBuffer& buffer, const Host& dest, bool donotblock = false);

            // TCP P2P SECTION:

            // Connect this socket to a remote Host. If hostname field is not empty and
            // not yet resolved, this method will call Host::resolve() internally.
            // If the timeout argument is invalid, the method will try to connect for ever.
            // If the Socket is a TLS socket, one party must force the server mode in
            // the TLS_init() call before calling this method
            OperationResult connectP2P(const Host& dest, const time::TimeFrame& timeout = time::TimeFrame());

            // TCP (or derived) SECTION:

            // Mark the socket as a listening socket, so it can accept() new connections.
            // If the instance of the socket was returned by an accept or the socket transport is not TCP,
            // this method will return SO_Unavailable
            OperationResult listen(size_t maxconn = 0);

            // Block until a new connection is available and return the new socket.
            // Return the requesting peer as an Host object
            // If the instance of the socket was returned by an accept or the socket transport is not TCP,
            // this method will return an invalid socket.
            // If this instance is configured with TLS, the returned Socket will inherit the basic init settings used in
            // the TLS_init() call previously
            Socket accept(Host& peer);

            // Block until a new connection is available and return the new socket
            // If the instance of the socket was returned by an accept or the socket transport is not TCP,
            // this method will return an invalid socket.
            // If this instance is configured with TLS, the returned Socket will inherit the basic init settings used in
            // the TLS_init() call previously
            Socket accept();

#ifdef LINUX_SYSTEM
            // If cork == true then stop sending out frames with send()
            // If cork == false then send all the outgoing queue
            // This is useful to compose a buffer using multiple send() calls and then
            // send all the buffer at once calling setCork(false)
            OperationResult setCork(bool cork = true);
#endif
            // Set the Nagle algorithm state. As default, TCP sockets have it enabled
            OperationResult useNagle(bool use = true);

            // Set the connection timeout of the socket. Default is 20 minutes (system dependent)
            // A value of zero corresponds to the system default
            OperationResult setTimeout(uint32_t timeout);

            // Enable keep alive probes usage.
            // When the connection goes idle and <idleTime> passed, TCP will start sending
            // keep alive probes every <interval> seconds to know if the counterpart is still connected.
            // TCP will drop the connection when <maxprobes> keep alive messages did not receive a response.
            // maxprobes is the maximum number of failed probes, idleTime and interval are expressed in seconds
            OperationResult useKeepAlive(bool use, int maxprobes, int idleTime, int interval);

            // Like the method above but leaving parameters unchanged
            OperationResult useKeepAlive(bool use = true);

            // Send out a file. This method is optimized and the buffer copy procedure
            // happens in kernel space, so it's really fast and efficient.
            // The file has to be open before this call
            OperationResult send(const data::filesystem::File& file);

            // Receive a file.
            // This method blocks until a file is received or timeout is reached.
            // -If timeout is not specified and no data are ready to be received, this call could block for ever
            // -If timeout (or/and cycTimeout) is specified, the behavior is the same as the recv() call
            OperationResult recv(data::filesystem::File& file, const time::TimeFrame& timeout = time::TimeFrame(), const time::TimeFrame& cycTimeout = time::TimeFrame());
        };
    }  // namespace network
}  // namespace cerberus

#endif  // CERBERUS_NETWORK_SOCKET_H
