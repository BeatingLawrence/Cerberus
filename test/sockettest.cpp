#include <cerberus/cerberus.h>
#include <cerberus/socket/tcpp2psocket.h>
#include <cerberus/socket/tcpsocket.h>
#include <cerberus/socket/udpsocket.h>
#include <gtest/gtest.h>

#include "cerberus/thread/thread.h"
#include "src/data/bytebuffer.h"

static int testCallback_UDP(cerberus::message::cerberus_message msg, cerberus::thread::Thread* thread)
{
    debug("receiver thread routine entered");
    cerberus::socket::UdpSocket socket;
    cerberus::Host host("127.0.0.1:22012");
    socket.bind(host);
    cerberus::data::ByteBuffer buf;
    cerberus::data::ByteBuffer exp("Hello, World!");
    socket.recv(buf);

    if (buf == exp)
    {
        return 101010;
    }
    else
    {
        return 1;
    }
}

static int testCallback_TCP(cerberus::message::cerberus_message msg, cerberus::thread::Thread* thread)
{
    debug("receiver thread routine entered");
    cerberus::socket::TcpSocket socket;
    socket.bind(cerberus::Host("127.0.0.1:33333"));
    socket.listen(3);
    cerberus::data::ByteBuffer buf;
    cerberus::data::ByteBuffer exp("Hello, World!");
    cerberus::Host h;
    auto s = socket.accept(h);
    s.setRecvBufferSize(exp.size());
    debug("accepted from, %s", h.toString().c_str());
    if (s.isFailed())
    {
        debug("accept error");
        return 1;
    }
    s.recv(buf);

    debug("received");

    if (buf == exp)
    {
        return 101010;
    }
    else
    {
        return 1;
    }
}

static int testCallback_TCP_P2P(cerberus::message::cerberus_message msg, cerberus::thread::Thread* thread)
{
    debug("receiver thread routine entered");
    cerberus::socket::TcpP2PSocket socket;
    socket.bind(cerberus::Host("127.0.0.1:44444"));

    if (socket.connect(cerberus::Host("127.0.0.1:55555"), 2000) != cerberus::SO_OK)
    {
        debug("connect error");
        return 1;
    }

    cerberus::data::ByteBuffer buf;
    cerberus::data::ByteBuffer exp("Hello, World!");
    socket.setRecvBufferSize(exp.size());

    if (socket.recv(buf) == cerberus::SO_OK)
    {
        debug("received");
    }

    if (buf == exp)
    {
        return 101010;
    }
    else
    {
        return 1;
    }
}

TEST(socketTest, UDP)
{
    // creating receiver thread
    cerberus::thread::Thread receiver("receiverTestThread", cerberus::thread::Thread::TP_OneShot);
    receiver.provideTickCallback(&testCallback_UDP);
    receiver.start();
    cerberus::thread::Thread::sleep(10);  // sleep THIS thread
    //
    cerberus::socket::UdpSocket socket;
    cerberus::data::ByteBuffer buf("Hello, World!");
    cerberus::Host host("127.0.0.1:22012");
    socket.sendTo(buf, host);
    EXPECT_EQ(receiver.join(), 101010);
}

TEST(socketTest, TCP)
{
    // creating receiver thread
    cerberus::thread::Thread receiver("receiverTestThread", cerberus::thread::Thread::TP_OneShot);
    receiver.provideTickCallback(&testCallback_TCP);
    receiver.start();
    cerberus::thread::Thread::sleep(10);  // sleep THIS thread
    //
    cerberus::socket::TcpSocket socket;
    ASSERT_EQ(socket.connect(cerberus::Host("127.0.0.1:33333")), cerberus::SocketOperation::SO_OK);
    cerberus::data::ByteBuffer buf("Hello, World!");
    ASSERT_EQ(socket.send(buf), cerberus::SocketOperation::SO_OK);
    EXPECT_EQ(receiver.join(), 101010);
}

TEST(socketTest, TCP_P2P)
{
    // creating receiver thread
    cerberus::thread::Thread receiver("receiverTestThread", cerberus::thread::Thread::TP_OneShot);
    receiver.provideTickCallback(&testCallback_TCP_P2P);
    receiver.start();
    cerberus::thread::Thread::sleep(10);  // sleep THIS thread
    //
    cerberus::socket::TcpP2PSocket socket;
    ASSERT_EQ(socket.bind(cerberus::Host("127.0.0.1:55555")), cerberus::SocketOperation::SO_OK);
    ASSERT_EQ(socket.connect(cerberus::Host("127.0.0.1:44444"), 2000), cerberus::SO_OK);
    cerberus::data::ByteBuffer buf("Hello, World!");
    ASSERT_EQ(socket.send(buf), cerberus::SocketOperation::SO_OK);
    EXPECT_EQ(receiver.join(), 101010);
}
