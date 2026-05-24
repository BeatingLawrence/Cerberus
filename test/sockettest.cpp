#include <cerberus.h>
#include <data/filesystem/file.h>
#include <gtest/gtest.h>
#include <network/httpclient.h>
#include <network/socket.h>
#include <thread/thread.h>

#define THREAD_ERROR 1
#define THREAD_SUCCESS 304050

using namespace crb;

static int testCallback_UDP(msg_ptr msg, Thread* thread)
{
    logDebug("receiver thread routine entered");
    auto socket = UDPSocket();
    crb::Host host("localhost:22012");
    socket.bind(host);
    ByteBuffer buf;
    ByteBuffer exp("Hello, World!");
    logDebug("receiving");
    socket.recv(buf);
    logDebug("received");
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

static int testCallback_TCP(msg_ptr msg, Thread* thread)
{
    logDebug("receiver thread routine entered");
    auto socket = TCPSocket();
    socket.bind(crb::Host("localhost:33333"));
    socket.listen(3);
    ByteBuffer buf;
    ByteBuffer exp("Hello, World!");
    auto s = socket.accept();
    if (!s.get())
    {
        logDebug("accept fail");
        socket.close();
        return THREAD_ERROR;
    }

    logDebug("accepted from, %s", s->remote().toString().c_str());
    s->setRecvBufferSize(exp.size());
    if (s->isFailed())
    {
        logDebug("accept error");
        socket.close();
        return THREAD_ERROR;
    }
    s->recv(buf);

    logDebug("received");

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

static int testCallback_TCP_P2P(msg_ptr msg, Thread* thread)
{
    logDebug("receiver thread routine entered");
    auto socket = TCPP2PSocket();
    socket.bind(crb::Host("localhost:44444"));

    if (socket.connectP2P(Host("localhost:57829"), 2000).fail("connect error in thread callback"))
    {
        logDebug("connect error");
        socket.close();
        return THREAD_ERROR;
    }

    ByteBuffer buf;
    ByteBuffer exp("Hello, World!");
    socket.setRecvBufferSize(exp.size());

    if (socket.recv(buf).ok())
    {
        logDebug("received");
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

static int testCallback_FTP(msg_ptr msg, Thread* thread)
{
    logDebug("receiver thread routine entered");
    auto socket = TCPSocket();
    socket.bind("localhost:54321");
    socket.listen(3);
    File file("ftp_socket_test_file_received.file", crb::FOM_ReadWriteTrunc);
    if (file.open().fail())
    {
        logDebug("file open error");
        socket.close();
        return THREAD_ERROR;
    }
    auto s = socket.accept();
    s->setRecvBufferSize(10);
    logDebug("accepted");
    if (s->isFailed())
    {
        logDebug("accept error");
        socket.close();
        return THREAD_ERROR;
    }
    auto ret = s->recv(file, 500);  // 0.5 seconds timeout

    if (ret.fail() && ret.res != crb::OR_Hangup)
    {
        socket.close();
        return THREAD_ERROR;
    }

    logDebug("received");

    file.close();
    socket.close();
    return THREAD_SUCCESS;
}

TEST(socketTest, UDP)
{
    // creating receiver thread
    Thread receiver(crb::TP_OneShot, crb::TimeFrame(10));
    receiver.checkIn("receiverTestThread_udp");
    receiver.provideTickCallback(&testCallback_UDP);
    receiver.start();
    Thread::sleep(10);  // sleep THIS thread
    //
    auto socket = UDPSocket();
    ByteBuffer buf("Hello, World!");
    socket.sendTo(buf, "localhost:22012");
    socket.close();
    EXPECT_EQ(receiver.join().expect().value, THREAD_SUCCESS);
}

TEST(socketTest, TCP)
{
    // creating receiver thread
    Thread receiver(crb::TP_OneShot, crb::TimeFrame(10));
    receiver.checkIn("receiverTestThread_tcp");
    receiver.provideTickCallback(&testCallback_TCP);
    receiver.start();
    Thread::sleep(10);  // sleep THIS thread
    //
    auto socket = TCPSocket();
    logDebug("connecting..");
    auto conn = socket.connect(crb::Host("localhost:33333"));
    if (conn.res != crb::OR_OK)
    {
        GTEST_SKIP() << "TCP connect not permitted in test environment";
    }
    logDebug("connected");
    ByteBuffer buf("Hello, World!");
    ASSERT_EQ(socket.send(buf).res, crb::OR_OK);
    socket.close();
    EXPECT_EQ(receiver.join().expect().value, THREAD_SUCCESS);
}

TEST(socketTest, TCP_P2P)
{
    // creating receiver thread
    Thread receiver(crb::TP_OneShot, crb::TimeFrame(10));
    receiver.checkIn("receiverTestThread_p2p");
    receiver.provideTickCallback(&testCallback_TCP_P2P);
    receiver.start();
    Thread::sleep(10);  // sleep THIS thread
    //
    auto socket = TCPP2PSocket();
    if (socket.bind(crb::Host("localhost:57829")).res != crb::OR_OK)
    {
        GTEST_SKIP() << "TCP P2P bind not permitted in test environment";
    }
    if (!socket.connectP2P(crb::Host("localhost:44444"), 2000).ok())
    {
        GTEST_SKIP() << "connectP2P not permitted in test environment";
    }
    ByteBuffer buf("Hello, World!");
    ASSERT_EQ(socket.send(buf).res, crb::OR_OK);

    socket.close();
    EXPECT_EQ(receiver.join().expect().value, THREAD_SUCCESS);
}

TEST(socketTest, FTP)
{
    // creating receiver thread
    Thread receiver(crb::TP_OneShot, crb::TimeFrame(10));
    receiver.checkIn("receiverTestThread_ftp");
    receiver.provideTickCallback(&testCallback_FTP);
    receiver.start();
    Thread::sleep(10);  // sleep THIS thread
    //
    // file creation
    File f("ftp_socket_test_file_sent.file", crb::FOM_ReadWriteTrunc);
    ASSERT_TRUE(f.open().ok());
    for (int i = 0; i < 500; i++)
    {
        f.writeLine("Hello, World!");
    }
    //
    auto socket = TCPSocket();
    ASSERT_EQ(socket.bind("localhost").res, crb::OR_OK);
    ASSERT_EQ(socket.connect("localhost:54321").res, crb::OR_OK);
    f.resetCursor();
    ASSERT_EQ(socket.send(f).res, crb::OR_OK);
    logDebug("FILE SENT");
    socket.close();  // it causes hangup on the other side
    EXPECT_EQ(receiver.join().expect().value, THREAD_SUCCESS);
    File rf("ftp_socket_test_file_received.file");
    ASSERT_TRUE(rf.open().ok());
    EXPECT_TRUE(rf.isEqual(f).value);

    rf.close();
    f.close();
}

TEST(socketTest, TLS_google)  // this test opens a TLS socket to google.com and gets the web page
{
    auto socket = TCPSocket();
    ASSERT_EQ(socket.TLS_init().res, crb::OR_OK);  // mark the socket as TLS
    socket.TLS_ignoreHangup(true).fail();
    logDebug("connecting..");
    crb::Host h("www.google.com:443");
    ASSERT_TRUE(socket.connect(h).ok("Internet socket tests require internet connection"));
    logDebug("connected with encryption: PROTO: %s CIPHER: %s", socket.TLS_getProtocolName().value.c_str(),
             socket.TLS_getCipherName().value.c_str());
    logDebug("sending get request");
    EXPECT_EQ(socket
                  .send("GET / HTTP/1.1\r\n"
                        "Host: www.google.com\r\n"
                        //"Accept-Encoding: identity"
                        //"Content-Length: 2048"
                        "Accept-Language: en-US,en;q=0.5\r\n"
                        "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:47.0) Gecko/20100101 "
                        "Firefox/47.0\r\n"
                        "Accept: text/html\r\n"
                        "Connection: keep-alive\r\n"
                        "Cache-Control: max-age=0\r\n\r\n")
                  .res,
              crb::OR_OK);
    ByteBuffer buf;
    socket.setRecvBufferSize(8192);
    logDebug("receiving");
    auto r = socket.recv_cyc(buf, 1500, 500);

    EXPECT_TRUE(r.ok("recv fail"));

    socket.close();

    // save the content in a file
    File f("received_http_data.txt", crb::FOM_ReadWriteTrunc);
    f.open();
    f.write(buf);
    f.close();
}

TEST(socketTest, HTTPClient)
{
    HTTPClient client;
    client.TLS_init();
    client.setRemote("www.google.com:443");
    HTTPRequest req;
    req.setup(crb::HTTP_GET, "/", crb::HTTP_1_1)
        .addHeaderField("Host", "www.google.com")
        .addHeaderField("Accept-Language", "en-US,en;q=0.5")
        .addHeaderField("User-Agent", "Mozilla/5.0 (X11; Linux x86_64; rv:47.0) Gecko/20100101 Firefox/47.0")
        .addHeaderField("Accept", "text/html")
        .addHeaderField("Connection", "keep-alive")
        .addHeaderField("Cache-Control", "max-age=0");
    //
    logDebug("data to be sent:");
    logDebug(req.data().toNormalizedString().value.c_str());
    //
    EXPECT_TRUE(client.makeRequest(req).ok("Internet socket tests require internet connection"));
    logDebug("request sent");
    auto response = client.getResponse(1000, 200);

    ASSERT_TRUE(response.ok("error while getting data"));

    EXPECT_TRUE(response.value.isOk());

    logDebug("received header lines:");
    for (int i = 0; i < response.value.getHeaderSize(); i++)
    {
        logDebug("%s: %s", response.value.getHeaderFieldName(i).c_str(),
                 response.value.getHeaderFieldValue(i).c_str());
    }

    // save the payload in a file
    File f("received_payload.txt", crb::FOM_ReadWriteTrunc);
    f.open();
    f.write(response.value.payload());
    f.close();

    logDebug("payload written on disk");
}
