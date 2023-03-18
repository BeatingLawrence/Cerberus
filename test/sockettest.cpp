#include "cerberus/thread/thread.h"
#include "src/data/bytebuffer.h"
#include <gtest/gtest.h>
#include <cerberus/cerberus.h>
#include <cerberus/socket/udpsocket.h>

static int testCallback(cerberus::message::cerberus_message msg, cerberus::thread::Thread* thread)
{
    debug("receiver thread routine entered");
    cerberus::socket::UdpSocket socket;
    cerberus::Host host;
    host.octect[0] = 127;
    host.octect[1] = 0;
    host.octect[2] = 0;
    host.octect[3] = 1;
    host.port = 22027;
    socket.bind(host);
    cerberus::data::ByteBuffer buf;
    cerberus::data::ByteBuffer exp("Hello, World!");
    buf.resize(exp.size());
    socket.recv(buf);

    if(buf == exp)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

TEST(socketTest, UDP)
{
    //creating receiver thread
    cerberus::thread::Thread receiver("receiverTestThread", cerberus::thread::Thread::TP_OneShot);
    receiver.provideTickCallback(&testCallback);
    receiver.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    //
    cerberus::socket::UdpSocket socket;
    cerberus::Host host;
    host.octect[0] = 127;
    host.octect[1] = 0;
    host.octect[2] = 0;
    host.octect[3] = 1;
    socket.bind(host);
    host.port = 22027;
    socket.connect(host);
    cerberus::data::ByteBuffer buf("Hello, World!");
    socket.send(buf);
    EXPECT_EQ(receiver.join(), 0);
}

