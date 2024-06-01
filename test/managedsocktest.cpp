#include <cerberus.h>
#include <gtest/gtest.h>
#include <network/socket.h>

using namespace cerberus;

static CHANDLE udp_sock_handle = 0;
static CHANDLE tcp_sock_handle = 0;

static int thrcb(cerberus_message msg, Thread* thread)
{
    logInfo("packet received from %s", msg->getSlot("host")->to<HostSlot>()->value().toString().c_str());
    logInfo("please verify content: %s",
            msg->getSlot("buffer")->to<BufferSlot>()->value().toString().c_str());

    return 0;
}

TEST(managedSockTest, creation_udp)
{
    SockSettings settings = {};
    settings.bind         = "127.0.0.1:8765";
    settings.transferMode = SockTransfer::Transfer_Bytes;
    settings.maxpayload   = 50;
    settings.type         = SocketType::Socket_UDP;

    auto s = Cerberus::newSock(settings);

    ASSERT_TRUE(s.ok("error in UDP sock creation"));

    udp_sock_handle = s.value;
}

TEST(managedSockTest, creation_tcp_server)
{
    SockSettings settings = {};
    settings.bind         = "127.0.0.1:8858";
    settings.transferMode = SockTransfer::Transfer_Time;
    settings.maxpayload   = 100;
    settings.type         = SocketType::Socket_TCP;
    settings.maxconn      = 2;
    settings.tout         = TimeFrame(500);
    settings.server       = true;

    auto s = Cerberus::newSock(settings);

    ASSERT_TRUE(s.ok("error in TCP sock creation"));

    tcp_sock_handle = s.value;
}

TEST(managedSockTest, udp)
{
    Thread t(TP_Message, "managsocktestthr");
    t.checkIn();
    t.provideTickCallback(thrcb);
    t.start();

    EXPECT_TRUE(Cerberus::addSockListener(udp_sock_handle, t.id()).ok());

    Thread::sleep(50);

    // send the packet

    ByteBuffer bb("Hello, world!");
    Socket sock(SocketType::Socket_UDP);
    EXPECT_TRUE(sock.sendTo(bb, "127.0.0.1:8765").ok());

    Thread::sleep(50);

    t.join(true);
    EXPECT_TRUE(Cerberus::removeSock(udp_sock_handle).ok());
}

TEST(managedSockTest, tcp)
{
    Thread t(TP_Message, "managsocktestthr");
    t.checkIn();
    t.provideTickCallback(thrcb);
    t.start();

    EXPECT_TRUE(Cerberus::addSockListener(tcp_sock_handle, t.id()).ok("fail"));

    Thread::sleep(50);

    // send the packet

    ByteBuffer bb;
    for (int i = 0; i < 100; i++)
    {
        bb.append("Hello, world!");
    }

    Socket sock(SocketType::Socket_TCP);
    EXPECT_TRUE(sock.connect("127.0.0.1:8858").ok());
    EXPECT_TRUE(sock.send(bb).ok());
    sock.close();

    Thread::sleep(500);

    t.join(true);
    EXPECT_TRUE(Cerberus::removeSock(tcp_sock_handle).ok("fail"));
}
