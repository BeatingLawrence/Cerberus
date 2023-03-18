#ifndef CERBERUS_SOCKET_SOCKETBASE_H
#define CERBERUS_SOCKET_SOCKETBASE_H

#include "../types.h"
#include "../Cerberus_global.h"

namespace cerberus
{
    namespace data
    {
        class ByteBuffer;
    }

    namespace socket
    {
        class CERBERUS_EXPORT SocketBase
        {
            private:
                SocketType m_type;

            protected:
                int m_fd;

            public:
                SocketBase() = delete;
                SocketBase(const SocketBase& other) = delete;
                SocketBase(SocketType type);

                virtual ~SocketBase();

                inline SocketType socketType()
                {
                    return m_type;
                }

                bool isFailed() const;

                SocketOperation resolve(Host& ip);

                SocketOperation bind(Host& interface);

                SocketOperation connect(Host& destination);

                SocketOperation close();

                SocketOperation send(const data::ByteBuffer& buffer, bool donotblock = false);

                SocketOperation recv(data::ByteBuffer& buffer, bool donotblock = false);
        };
    }
}

#endif // CERBERUS_SOCKET_SOCKETBASE_H

