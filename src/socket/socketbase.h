#ifndef CERBERUS_SOCKET_SOCKETBASE_H
#define CERBERUS_SOCKET_SOCKETBASE_H

#include "../data/bytebuffer.h"
#include "../types.h"

namespace cerberus
{
    namespace data
    {
        class ByteBuffer;
    }

    namespace socket
    {
        class SocketBase
        {
           private:
            SocketType m_type;

           protected:
            SocketBase(SocketType type);

            int m_fd;

            data::ByteBuffer m_recvBuffer;

           public:
            SocketBase() = delete;
            SocketBase(const SocketBase& other) = delete;

            virtual ~SocketBase();

            inline SocketType socketType() { return m_type; }

            bool isFailed() const;

            void setRecvBufferSize(size_t size);

            SocketOperation resolve(Host& ip);

            SocketOperation bind(Host& interface);

            SocketOperation connect(Host& destination);

            SocketOperation close();

            SocketOperation send(const data::ByteBuffer& buffer, bool donotblock = false);

            SocketOperation recv(data::ByteBuffer& buffer, bool donotblock = false);
        };
    }  // namespace socket
}  // namespace cerberus

#endif  // CERBERUS_SOCKET_SOCKETBASE_H
