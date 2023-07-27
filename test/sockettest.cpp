#include <cerberus/cerberus.h>
#include <cerberus/socket/socket.h>
#include <gtest/gtest.h>

#include "cerberus/thread/thread.h"
#include "src/data/bytebuffer.h"

#define THREAD_ERROR 1
#define THREAD_SUCCESS 304050

static int testCallback_UDP(cerberus::message::cerberus_message msg, cerberus::thread::Thread* thread)
{
    debug("receiver thread routine entered");
    auto socket = UDPSocket("UDP receiver");
    cerberus::Host host("localhost:22012");
    socket.bind(host);
    cerberus::data::ByteBuffer buf;
    cerberus::data::ByteBuffer exp("Hello, World!");
    debug("receiving");
    socket.recv(buf);
    debug("received");
    if (buf == exp)
    {
        return THREAD_SUCCESS;
    }
    else
    {
        return THREAD_ERROR;
    }
}

static int testCallback_TCP(cerberus::message::cerberus_message msg, cerberus::thread::Thread* thread)
{
    debug("receiver thread routine entered");
    auto socket = TCPSocket("TCP receiver");
    socket.bind(cerberus::Host("localhost:33333"));
    socket.listen(3);
    cerberus::data::ByteBuffer buf;
    cerberus::data::ByteBuffer exp("Hello, World!");
    cerberus::Host h;
    auto s = socket.accept(h);
    if (s.isFailed())
    {
        debug("accept fail");
        return THREAD_ERROR;
    }

    debug("accepted from, %s", h.toString().c_str());
    s.setRecvBufferSize(exp.size());
    if (s.isFailed())
    {
        debug("accept error");
        return THREAD_ERROR;
    }
    s.recv(buf);

    debug("received");

    if (buf == exp)
    {
        return THREAD_SUCCESS;
    }
    else
    {
        return THREAD_ERROR;
    }
}

static int testCallback_TCP_P2P(cerberus::message::cerberus_message msg, cerberus::thread::Thread* thread)
{
    debug("receiver thread routine entered");
    auto socket = TCPP2PSocket("TCPP2P receiver");
    socket.bind(cerberus::Host("localhost:44444"));

    if (socket.connectP2P(cerberus::Host("localhost:55555"), 2000) != cerberus::OR_OK)
    {
        debug("connect error");
        return THREAD_ERROR;
    }

    cerberus::data::ByteBuffer buf;
    cerberus::data::ByteBuffer exp("Hello, World!");
    socket.setRecvBufferSize(exp.size());

    if (socket.recv(buf) == cerberus::OR_OK)
    {
        debug("received");
    }

    if (buf == exp)
    {
        return THREAD_SUCCESS;
    }
    else
    {
        return THREAD_ERROR;
    }
}

static int testCallback_FTP(cerberus::message::cerberus_message msg, cerberus::thread::Thread* thread)
{
    debug("receiver thread routine entered");
    auto socket = FTPSocket("FTP receiver");
    socket.bind("localhost:54321");
    socket.listen(3);
    cerberus::data::filesystem::File file("ftp_socket_test_file_received.file", cerberus::FOM_ReadWriteTrunc);
    if (!file.open())
    {
        debug("file open error");
        return THREAD_ERROR;
    }
    auto s = socket.accept();
    s.setRecvBufferSize(10);
    debug("accepted");
    if (s.isFailed())
    {
        debug("accept error");
        return THREAD_ERROR;
    }
    auto ret = s.recv(file, 500);  // 0.5 seconds timeout

    if (ret != cerberus::OR_OK)
    {
        debug("receive error");
        return THREAD_ERROR;
    }

    debug("received");

    file.close();

    return THREAD_SUCCESS;
}

TEST(socketTest, UDP)
{
    // creating receiver thread
    cerberus::thread::Thread receiver("receiverTestThread", cerberus::thread::Thread::TP_OneShot);
    receiver.provideTickCallback(&testCallback_UDP);
    receiver.start();
    cerberus::thread::Thread::sleep(10);  // sleep THIS thread
    //
    auto socket = UDPSocket("UDP transmitter");
    cerberus::data::ByteBuffer buf("Hello, World!");
    socket.sendTo(buf, "localhost:22012");
    EXPECT_EQ(receiver.join(), THREAD_SUCCESS);
}

TEST(socketTest, TCP)
{
    // creating receiver thread
    cerberus::thread::Thread receiver("receiverTestThread", cerberus::thread::Thread::TP_OneShot);
    receiver.provideTickCallback(&testCallback_TCP);
    receiver.start();
    cerberus::thread::Thread::sleep(10);  // sleep THIS thread
    //
    auto socket = TCPSocket("TCP transmitter");
    debug("connecting..");
    ASSERT_EQ(socket.connect(cerberus::Host("localhost:33333")).res, cerberus::OR_OK);
    debug("connected");
    cerberus::data::ByteBuffer buf("Hello, World!");
    ASSERT_EQ(socket.send(buf).res, cerberus::OR_OK);
    debug("sent");
    EXPECT_EQ(receiver.join(), THREAD_SUCCESS);
}

TEST(socketTest, TCP_P2P)
{
    // creating receiver thread
    cerberus::thread::Thread receiver("receiverTestThread", cerberus::thread::Thread::TP_OneShot);
    receiver.provideTickCallback(&testCallback_TCP_P2P);
    receiver.start();
    cerberus::thread::Thread::sleep(10);  // sleep THIS thread
    //
    auto socket = TCPP2PSocket("TCPP2P transmitter");
    ASSERT_EQ(socket.bind(cerberus::Host("localhost:55555")).res, cerberus::OR_OK);
    ASSERT_EQ(socket.connectP2P(cerberus::Host("localhost:44444"), 2000).res, cerberus::OR_OK);
    cerberus::data::ByteBuffer buf("Hello, World!");
    ASSERT_EQ(socket.send(buf).res, cerberus::OR_OK);
    EXPECT_EQ(receiver.join(), THREAD_SUCCESS);
}

TEST(socketTest, FTP)
{
    // creating receiver thread
    cerberus::thread::Thread receiver("receiverTestThread", cerberus::thread::Thread::TP_OneShot);
    receiver.provideTickCallback(&testCallback_FTP);
    receiver.start();
    cerberus::thread::Thread::sleep(10);  // sleep THIS thread
    //
    // file creation
    cerberus::data::filesystem::File f("ftp_socket_test_file_sent.file", cerberus::FOM_ReadWriteTrunc);
    ASSERT_TRUE(f.open());
    for (int i = 0; i < 500; i++)
    {
        f.writeLine("Hello, World!");
    }
    //
    auto socket = FTPSocket("FTP transmitter");
    ASSERT_EQ(socket.bind("localhost").res, cerberus::OR_OK);
    ASSERT_EQ(socket.connect("localhost:54321").res, cerberus::OR_OK);
    f.resetCursor();
    ASSERT_EQ(socket.send(f).res, cerberus::OR_OK);
    debug("FILE SENT");
    socket.close();
    EXPECT_EQ(receiver.join(), THREAD_SUCCESS);
    cerberus::data::filesystem::File rf("ftp_socket_test_file_received.file");
    ASSERT_TRUE(rf.open());
    EXPECT_TRUE(rf.isEqual(f));

    rf.close();
    f.close();
}
