#ifndef CERBERUS_SOCKET_TCPP2PSOCKET_H
#define CERBERUS_SOCKET_TCPP2PSOCKET_H

#include "../time/time.h"
#include "socketbase.h"

namespace cerberus
{
    namespace socket
    {
        class TcpP2PSocket : public cerberus::socket::SocketBase
        {
           public:
            TcpP2PSocket();

            SocketOperation connect(const Host& destination, const time::Time& timeout);
        };
    }  // namespace socket
}  // namespace cerberus

#endif  // CERBERUS_SOCKET_TCPP2PSOCKET_H
