#ifndef CERBERUS_SOCKET_SOCKET_H
#define CERBERUS_SOCKET_SOCKET_H

#include "../types.h"
#include "src/core/cerberusobject.h"
#include "src/data/bytebuffer.h"
#include "src/time/time.h"

#define UDPSocket(namestr) cerberus::socket::Socket(cerberus::CerberusObject::Socket_UDP, namestr)
#define TCPSocket(namestr) cerberus::socket::Socket(cerberus::CerberusObject::Socket_TCP, namestr)
#define TCPP2PSocket(namestr) cerberus::socket::Socket(cerberus::CerberusObject::Socket_TCPP2P, namestr)
#define FTPSocket(namestr) cerberus::socket::Socket(cerberus::CerberusObject::Socket_FTP, namestr)

// A socket capable of great things!

// This is the Cerberus Socket class. It defines a socket usable in all circumstances.
// This class implements stream socket as well as datagram, HTTP, FTP, ICMP sockets and so on.
// The socket type mus be specified in the constructor

namespace cerberus
{
    namespace data
    {
        namespace filesystem
        {
            class File;
        }
    }  // namespace data

    namespace socket
    {
        class Socket : public CerberusObject
        {
           private:
            Socket(SocketType type, int fd);

            // This method creates a datagram socket and assigns the resulting file descriptor to m_fd.
            // Check is the socket isFailed() after this call
            void createUdpSocket();

            // This method creates a stream socket and assigns the resulting file descriptor to m_fd.
            // Check is the socket isFailed() after this call
            void createTcpSocket();

            int _accept(Host& peer);

            bool m_extern;  // used for acceptable sockets

            size_t m_maxConnections;  // used for acceptable sockets

            int m_fd;

            data::ByteBuffer m_recvBuffer;

           public:
            Socket() = delete;

            Socket(const Socket& other) = delete;

            Socket(SocketType type, const std::string& name = std::string());

            virtual ~Socket();

            // GENERAL SECTION:

            // Mark the socket as a listening socket, so it can accept() new connections.
            // If the instance of the socket was returned by an accept, this method will return SO_Unavailable
            OperationResult listen(size_t maxconn = 0);

            // Block until a new connection is available and return the new socket.
            // Return the requesting peer as an Host object
            // If the instance of the socket was returned by an accept, this method will return SO_Unavailable
            Socket accept(Host& peer);

            // Block until a new connection is available and return the new socket
            // If the instance of the socket was returned by an accept, this method will return SO_Unavailable
            Socket accept();

            // Binds this socket to a given interface
            OperationResult bind(const Host& iface);

            // Connects this socket to a remote Host. If hostname field is not empty and
            // not yet resolved, this method will call Host::resolve() internally
            OperationResult connect(const Host& dest);

            // Send out a buffer
            OperationResult send(const data::ByteBuffer& buffer, bool donotblock = false);

            // Receive a buffer
            OperationResult recv(data::ByteBuffer& buffer, bool donotblock = false);

            // Try to receive a buffer for a timeout time. If timeout is an invalid time an error will be returned
            OperationResult recv(data::ByteBuffer& buffer, const time::Time& timeout);

            bool isFailed() const;

            // Set the buffer size used for recv calls, default is 512 bytes
            void setRecvBufferSize(size_t size);

            // Sets the maximum incoming connections number. If the queue is full, new peers
            // will receive a connection refused error when they do a connect()
            void setMaxConnections(size_t maxconn);

            // Close the socket
            OperationResult close();

            // UDP SECTION:

            OperationResult sendTo(const data::ByteBuffer& buffer, const Host& dest, bool donotblock = false);

            // TCP SECTION:

            // If cork == true then stop sending out frames with send()
            // If cork == false then send all the outgoing queue
            // This is useful to compose a buffer using multiple send() calls and then
            // send all the buffer at once calling setCork(false)
            OperationResult setCork(bool cork = true);

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

            // FTP SECTION:

            // Send out a file. This method is optimized and the buffer copy procedure
            // happens in kernel space, so it's really fast and efficient.
            // The file has to be open before this call
            OperationResult send(const data::filesystem::File& file);

            // Receive a file.
            // This method blocks until a file is received or timeout is reached. If timeout is not specified
            // and no data are ready to be received, this call blocks forever
            OperationResult recv(data::filesystem::File& file, const time::Time& timeout = time::Time());

            // TCP P2P SECTION:

            OperationResult connectP2P(const Host& dest, const time::Time& timeout = time::Time());
        };
    }  // namespace socket
}  // namespace cerberus

#endif  // CERBERUS_SOCKET_SOCKETBASE_H
