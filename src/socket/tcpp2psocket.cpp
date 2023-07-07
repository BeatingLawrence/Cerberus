#include "tcpp2psocket.h"

#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/socket.h>

#include "../core/cerberuslog.h"
#include "../time/timer.h"

using namespace cerberus::socket;

//=============================================================================
TcpP2PSocket::TcpP2PSocket()
    : SocketBase(Socket_TCPP2P)
{
    m_fd = ::socket(AF_INET, SOCK_STREAM, 0);

    if (m_fd == -1)
    {
        debug("error in socket creation: %s", strerror(errno));
    }
}
//=============================================================================
cerberus::SocketOperation TcpP2PSocket::connect(const Host &dest, const time::Time &timeout)
{
    if (isFailed())
    {
        return SocketOperation::SO_FailedSocket;
    }

    Host h = dest;

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(h.port);

    if (!h.hostname.empty() && !h.resolved)
    {
        resolve(h);
    }

    if (h.octet_networkOrder == 0)
    {
        addr.sin_addr.s_addr = INADDR_ANY;
    }
    else
    {
        addr.sin_addr.s_addr = h.octet_networkOrder;
    }

    time::Timer timer(timeout);
    timer.start();
    int ret = -1;

    while (timer.isRunning())
    {
        ret = ::connect(m_fd, (sockaddr *)&addr, sizeof(sockaddr_in));

        if (ret == 0 || (ret == -1 && errno != ECONNREFUSED))
        {
            break;
        }
    }

    if (ret == -1)
    {
        debug("error in socket connect: %s", strerror(errno));
        return SO_ConnectFailure;
    }

    return SO_OK;
}
//=============================================================================
