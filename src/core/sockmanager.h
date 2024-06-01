#ifndef SOCKMANAGER_H
#define SOCKMANAGER_H

#include "../mutex/mutex.h"
#include "../network/socket.h"

namespace cerberus
{
    class SockManager
    {
       public:
        struct SockData
        {
            Socket* sock;
            SockSettings settings;

            CHANDLE handle;
            bool remove;
            Mutex mutex;

            std::list<HASH32> threads;

            SockData() = delete;

            SockData(SocketType type)
                : sock(new Socket(type)),
                  handle(0),
                  remove(false) {};

            SockData(Socket* sock)
                : sock(sock),
                  handle(0),
                  remove(false) {};

            ~SockData() { delete sock; };
        };

       private:
        std::list<SockData*> m_sockets;
        Mutex m_mutex;

        SockData* createNewSocket(SocketType type);

       public:
        SockManager();

        ~SockManager();

        OpResData<SockData*> newSock(const SockSettings& settings);

        // this method is usable only by the owner thread of the socket
        OpRes removeSock(SockData* sock);

        OpRes addListener(CHANDLE sock, HASH32 threadID);

        SockData* newSockCopy(const SockSettings& settings, Socket* sock, SockData* parentData);

        OpResData<SockData*> getSock(CHANDLE sock);

        OpRes sockSend(CHANDLE sock, const ByteBuffer& buffer);

        OpRes scheduleRemoval(CHANDLE sock);

        void scheduleRemoval_all();

        void clear();
    };
}  // namespace cerberus

#endif  // SOCKMANAGER_H
