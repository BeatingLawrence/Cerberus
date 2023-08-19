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
    socket.close();
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
        socket.close();
        return THREAD_ERROR;
    }

    debug("accepted from, %s", h.toString().c_str());
    s.setRecvBufferSize(exp.size());
    if (s.isFailed())
    {
        debug("accept error");
        socket.close();
        return THREAD_ERROR;
    }
    s.recv(buf);

    debug("received");

    socket.close();

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
        socket.close();
        return THREAD_ERROR;
    }

    cerberus::data::ByteBuffer buf;
    cerberus::data::ByteBuffer exp("Hello, World!");
    socket.setRecvBufferSize(exp.size());

    if (socket.recv(buf) == cerberus::OR_OK)
    {
        debug("received");
    }

    socket.close();

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
    auto socket = TCPSocket("FTP receiver");
    socket.bind("localhost:54321");
    socket.listen(3);
    cerberus::data::filesystem::File file("ftp_socket_test_file_received.file", cerberus::FOM_ReadWriteTrunc);
    if (!file.open())
    {
        debug("file open error");
        socket.close();
        return THREAD_ERROR;
    }
    auto s = socket.accept();
    s.setRecvBufferSize(10);
    debug("accepted");
    if (s.isFailed())
    {
        debug("accept error");
        socket.close();
        return THREAD_ERROR;
    }
    auto ret = s.recv(file, 500);  // 0.5 seconds timeout

    if (ret != cerberus::OR_OK)
    {
        debug("receive error %i", ret.res);
        socket.close();
        return THREAD_ERROR;
    }

    debug("received");

    file.close();
    socket.close();
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
    socket.close();
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
    socket.close();
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

    socket.close();
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
    auto socket = TCPSocket("FTP transmitter");
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

TEST(socketTest, TLS_google)  // this test opens a TLS socket to google.com and gets the web page
{
    auto socket = TCPSocket("TLS socket");
    ASSERT_EQ(socket.TLS_init().res, cerberus::OR_OK);  // mark the socket as TLS
    socket.TLS_ignoreHangup(false).fail(true);
    debug("connecting..");
    cerberus::Host h("www.google.com");
    h.port = 443;
    ASSERT_EQ(socket.connect(h).res, cerberus::OR_OK);
    debug("connected with encryption: PROTO: %s CIPHER: %s", socket.TLS_getProtocolName().c_str(), socket.TLS_getCipherName().c_str());
    debug("sending get request");
    EXPECT_EQ(socket
                  .send("GET / HTTP/1.1\r\n"
                        "Host: www.google.com\r\n"
                        //"Accept-Encoding: identity"
                        //"Content-Length: 2048"
                        "Accept-Language: en-US,en;q=0.5\r\n"
                        "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:47.0) Gecko/20100101 Firefox/47.0\r\n"
                        "Accept: text/html\r\n"
                        "Connection: close\r\n"
                        "Cache-Control: max-age=0\r\n\r\n")
                  .res,
              cerberus::OR_OK);
    cerberus::data::ByteBuffer buf;
    socket.setRecvBufferSize(8192);
    debug("receiving");
    auto r = socket.recvAll(buf).res;

    debug("Checking shutdown state");

    auto sh = socket.TLS_getShutdown();

    if (sh.b1)
    {
        debug("SHUTDOWN SENT");
    }

    if (sh.b2)
    {
        debug("SHUTDOWN RECEIVED");
    }

    EXPECT_TRUE((r == cerberus::OR_OK) || (r == cerberus::OR_TimedOut));

    socket.close();

    // save the content in a file
    cerberus::data::filesystem::File f("received_http_data.txt", cerberus::FOM_ReadWriteTrunc);
    f.open();
    f.write(buf);
    f.close();
}
