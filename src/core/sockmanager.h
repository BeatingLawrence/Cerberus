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
            enum SockStatus : uint8_t
            {
                Using,
                Halted,
                Remove,
            } status;

            Socket* sock;
            SockSettings settings;

            CHANDLE handle;
            Mutex mutex;

            std::list<HASH32> threads;

            SockData() = delete;

            SockData(SocketType type)
                : sock(new Socket(type)){};

            SockData(Socket* sock)
                : sock(sock){};

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

        OpRes removeSock(SockData* sock);

        OpRes addListener(CHANDLE sock, HASH32 threadID);

        SockData* addTcp(const SockSettings& settings, Socket* sock);

        OpResData<SockData*> getSock(CHANDLE sock);
    };
}  // namespace cerberus

#endif  // SOCKMANAGER_H
