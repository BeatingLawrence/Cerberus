#ifndef CERBERUS_SOCKET_UDPSOCKET_H
#define CERBERUS_SOCKET_UDPSOCKET_H

#include "socketbase.h"

namespace cerberus
{
    namespace socket
    {
        class UdpSocket : public cerberus::socket::SocketBase
        {
           public:
            UdpSocket();

            virtual ~UdpSocket();

            SocketOperation sendTo(const data::ByteBuffer& buffer, const Host& dest, bool donotblock = false);
        };
    }  // namespace socket
}  // namespace cerberus

#endif  // CERBERUS_SOCKET_UDPSOCKET_H
