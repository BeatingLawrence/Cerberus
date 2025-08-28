#include <cerberus.h>
#include <gtest/gtest.h>
#include <network/socket.h>

using namespace cerberus;

static CHANDLE udp_socket_handle = 0;
static CHANDLE tcp_socket_handle = 0;

static int thrcb(msg_ptr msg, Thread* thread)
{
    logInfo("packet received from %s", msg->getSlot("host")->to<HostSlot>()->value().toString().c_str());
    logInfo("please verify content: %s",
            msg->getSlot("buffer")->to<BufferSlot>()->value().toString().c_str());

    return 0;
}

TEST(managedSocketTest, creation_udp)
{
    SocketSettings settings = {};
    settings.bind           = "127.0.0.1:8765";
    settings.transferMode   = SocketTransfer::Transfer_Bytes;
    settings.maxpayload     = 50;
    settings.type           = SocketType::Socket_UDP;

    auto s = Cerberus::newSocket(settings);

    ASSERT_TRUE(s.ok("error in UDP sock creation"));

    udp_socket_handle = s.value;
}

TEST(managedSocketTest, creation_tcp_server)
{
    SocketSettings settings = {};
    settings.bind           = "127.0.0.1:8858";
    settings.transferMode   = SocketTransfer::Transfer_Time;
    settings.maxpayload     = 100;
    settings.type           = SocketType::Socket_TCP;
    settings.maxconn        = 2;
    settings.tout           = TimeFrame(500);
    settings.server         = true;

    auto s = Cerberus::newSocket(settings);

    ASSERT_TRUE(s.ok("error in TCP socket creation"));

    tcp_socket_handle = s.value;
}

TEST(managedSocketTest, udp)
{
    Thread t(TP_Message, "managsockettestthr");
    t.checkIn();
    t.provideTickCallback(thrcb);
    t.start();

    EXPECT_TRUE(Cerberus::addSocketListener(udp_socket_handle, t.id()).ok());

    Thread::sleep(50);

    // send the packet

    ByteBuffer bb("Hello, world!");
    Socket socket(SocketType::Socket_UDP);
    EXPECT_TRUE(socket.sendTo(bb, "127.0.0.1:8765").ok());

    Thread::sleep(50);

    t.join(true);
    EXPECT_TRUE(Cerberus::removeSocket(udp_socket_handle).ok());
}

TEST(managedSocketTest, tcp)
{
    Thread t(TP_Message, "managsockettestthr");
    t.checkIn();
    t.provideTickCallback(thrcb);
    t.start();

    EXPECT_TRUE(Cerberus::addSocketListener(tcp_socket_handle, t.id()).ok("fail"));

    Thread::sleep(50);

    // send the packet

    ByteBuffer bb;
    for (int i = 0; i < 100; i++)
    {
        bb.append("Hello, world!");
    }

    Socket socket(SocketType::Socket_TCP);
    EXPECT_TRUE(socket.connect("127.0.0.1:8858").ok());
    EXPECT_TRUE(socket.send(bb).ok());
    socket.close();

    Thread::sleep(500);

    t.join(true);
    EXPECT_TRUE(Cerberus::removeSocket(tcp_socket_handle).ok("fail"));
}
