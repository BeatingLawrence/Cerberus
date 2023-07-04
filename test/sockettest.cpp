#include <cerberus/cerberus.h>
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
    debug("accepted, %s", h.toString().c_str());
    if (s.isFailed())
    {
        debug("accept error");
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

TEST(socketTest, TCP)
{
    // creating receiver thread
    cerberus::thread::Thread receiver("receiverTestThread", cerberus::thread::Thread::TP_OneShot);
    receiver.provideTickCallback(&testCallback_TCP);
    receiver.start();
    debug("receiver thread started");
    cerberus::thread::Thread::sleep(10);  // sleep THIS thread
    //
    cerberus::socket::TcpSocket socket;
    ASSERT_EQ(socket.connect(cerberus::Host("127.0.0.1:33333")), cerberus::SocketOperation::SO_OK);
    debug("transmitter connected");
    cerberus::data::ByteBuffer buf("Hello, World!");
    ASSERT_EQ(socket.send(buf), cerberus::SocketOperation::SO_OK);
    debug("transmitter buffer sent");
    socket.close();
    EXPECT_EQ(receiver.join(), 101010);
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
    cerberus::Host host("127.0.0.1:22012");
    socket.connect(host);
    cerberus::data::ByteBuffer buf("Hello, World!");
    socket.send(buf);
    EXPECT_EQ(receiver.join(), 101010);
}
