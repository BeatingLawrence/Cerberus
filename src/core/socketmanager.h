#ifndef SOCKETMANAGER_H
#define SOCKETMANAGER_H

#include "../network/socket.h"
#include "../thread/mutex.h"

namespace cerberus
{
    class SocketManager
    {
       public:
        struct SocketData
        {
            Socket* s;
            SocketSettings settings;

            CHANDLE handle;
            bool remove;
            Mutex mutex;

            std::list<HASH32> threads;

            SocketData() = delete;

            SocketData(SocketType type)
                : s(new Socket(type)),
                  handle(0),
                  remove(false) {};

            SocketData(Socket* socket)
                : s(socket),
                  handle(0),
                  remove(false) {};

            ~SocketData() { delete s; };
        };

       private:
        std::list<SocketData*> m_sockets;
        Mutex m_mutex;

        SocketData* createNewSocket(SocketType type);

       public:
        SocketManager();

        ~SocketManager();

        OpResData<SocketData*> newSocket(const SocketSettings& settings);

        // this method is usable only by the owner thread of the socket
        OpRes removeSocket(SocketData* socket);

        OpRes addListener(CHANDLE socket, HASH32 threadID);

        SocketData* newSocketCopy(const SocketSettings& settings, Socket* socket, SocketData* parentData);

        OpResData<SocketData*> getSocket(CHANDLE socket);

        OpRes socketSend(CHANDLE socket, const ByteBuffer& buffer);

        OpRes scheduleRemoval(CHANDLE socket);

        void scheduleRemoval_all();

        void clear();
    };
}  // namespace cerberus

#endif  // SOCKETMANAGER_H
