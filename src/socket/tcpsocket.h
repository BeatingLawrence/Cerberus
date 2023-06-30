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
            TcpSocket();

            void setMaxConnections(size_t maxconn);

            SocketOperation listen(size_t maxconn = 0);

            TcpSocket accept(Host& peer);

            TcpSocket accept();
        };
    }  // namespace socket
}  // namespace cerberus

#endif  // CERBERUS_SOCKET_TCPSOCKET_H
