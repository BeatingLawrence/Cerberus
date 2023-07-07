#ifndef CERBERUS_SOCKET_TCPSOCKET_H
#define CERBERUS_SOCKET_TCPSOCKET_H

#include "socketbase.h"

namespace cerberus
{
    namespace socket
    {
        class TcpSocket : public cerberus::socket::SocketBase
        {
           private:
            TcpSocket(int fd);

            size_t m_maxConnections;

           public:
            // Construct a TcpSocket object
            TcpSocket();

            // Sets the maximum incoming connections number. If the queue is full, new peers
            // will receive a connection refused error when they do a connect()
            void setMaxConnections(size_t maxconn);

            // Mark the socket as a listening socket, so it can accept() new connections
            SocketOperation listen(size_t maxconn = 0);

            // Block until a new connection is available and return the new socket.
            // Return the requesting peer as an Host object
            TcpSocket accept(Host& peer);

            // Block until a new connection is available and return the new socket
            TcpSocket accept();

            // If cork == true then stop sending out frames with send()
            // If cork == false then send all the outgoing queue
            // This is useful to compose a buffer using multiple send() calls and then
            // send all the buffer at once calling setCork(false)
            SocketOperation setCork(bool cork = true);

            // Set the Nagle algorithm state. As default, TCP sockets have it enabled
            SocketOperation useNagle(bool use = true);

            // Set the connection timeout of the socket. Default is 20 minutes (system dependent)
            // A value of zero corresponds to the system default
            SocketOperation setTimeout(uint32_t timeout);

            // Enable keep alive probes usage.
            // When the connection goes idle and <idleTime> passed, TCP will start sending
            // keep alive probes every <interval> seconds to know if the counterpart is still connected.
            // TCP will drop the connection when <maxprobes> keep alive messages did not receive a response.
            // maxprobes is the maximum number of failed probes, idleTime and interval are expressed in seconds
            SocketOperation useKeepAlive(bool use, int maxprobes, int idleTime, int interval);

            // Like the method above but leaving parameters unchanged
            SocketOperation useKeepAlive(bool use = true);
        };
    }  // namespace socket
}  // namespace cerberus

#endif  // CERBERUS_SOCKET_TCPSOCKET_H
