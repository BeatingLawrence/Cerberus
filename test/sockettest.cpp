#include <cerberus/cerberus.h>
#include <cerberus/network/httpclient.h>
#include <cerberus/network/socket.h>
#include <cerberus/thread/thread.h>
#include <gtest/gtest.h>

#include "cerberus/data/filesystem/file.h"

#define THREAD_ERROR 1
#define THREAD_SUCCESS 304050

static int testCallback_UDP(cerberus::cerberus_message msg, cerberus::thread::Thread* thread)
{
    logDebug("receiver thread routine entered");
    auto socket = UDPSocket("UDP receiver");
    cerberus::Host host("localhost:22012");
    socket.bind(host);
    cerberus::data::ByteBuffer buf;
    cerberus::data::ByteBuffer exp("Hello, World!");
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

static int testCallback_TCP(cerberus::cerberus_message msg, cerberus::thread::Thread* thread)
{
    logDebug("receiver thread routine entered");
    auto socket = TCPSocket("TCP receiver");
    socket.bind(cerberus::Host("localhost:33333"));
    socket.listen(3);
    cerberus::data::ByteBuffer buf;
    cerberus::data::ByteBuffer exp("Hello, World!");
    cerberus::Host h;
    auto s = socket.accept(h);
    if (s.isFailed())
    {
        logDebug("accept fail");
        socket.close();
        return THREAD_ERROR;
    }

    logDebug("accepted from, %s", h.toString().c_str());
    s.setRecvBufferSize(exp.size());
    if (s.isFailed())
    {
        logDebug("accept error");
        socket.close();
        return THREAD_ERROR;
    }
    s.recv(buf);

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

static int testCallback_TCP_P2P(cerberus::cerberus_message msg, cerberus::thread::Thread* thread)
{
    logDebug("receiver thread routine entered");
    auto socket = TCPP2PSocket("TCPP2P receiver");
    socket.bind(cerberus::Host("localhost:44444"));

    if (socket.connectP2P(cerberus::Host("localhost:57829"), 2000) != cerberus::OR_OK)
    {
        logDebug("connect error");
        socket.close();
        return THREAD_ERROR;
    }

    cerberus::data::ByteBuffer buf;
    cerberus::data::ByteBuffer exp("Hello, World!");
    socket.setRecvBufferSize(exp.size());

    if (socket.recv(buf) == cerberus::OR_OK)
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

static int testCallback_FTP(cerberus::cerberus_message msg, cerberus::thread::Thread* thread)
{
    logDebug("receiver thread routine entered");
    auto socket = TCPSocket("FTP receiver");
    socket.bind("localhost:54321");
    socket.listen(3);
    cerberus::data::filesystem::File file("ftp_socket_test_file_received.file", cerberus::FOM_ReadWriteTrunc);
    if (file.open().fail(true))
    {
        logDebug("file open error");
        socket.close();
        return THREAD_ERROR;
    }
    auto s = socket.accept();
    s.setRecvBufferSize(10);
    logDebug("accepted");
    if (s.isFailed())
    {
        logDebug("accept error");
        socket.close();
        return THREAD_ERROR;
    }
    auto ret = s.recv(file, 500);  // 0.5 seconds timeout

    if (ret.fail(true))
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
    cerberus::thread::Thread receiver(cerberus::TP_OneShot, "receiverTestThread");
    receiver.provideTickCallback(&testCallback_UDP);
    receiver.start();
    cerberus::thread::Thread::sleep(10);  // sleep THIS thread
    //
    auto socket = UDPSocket("UDP transmitter");
    cerberus::data::ByteBuffer buf("Hello, World!");
    socket.sendTo(buf, "localhost:22012");
    socket.close();
    EXPECT_EQ(receiver.join().expect().i, THREAD_SUCCESS);
}

TEST(socketTest, TCP)
{
    // creating receiver thread
    cerberus::thread::Thread receiver(cerberus::TP_OneShot, "receiverTestThread");
    receiver.provideTickCallback(&testCallback_TCP);
    receiver.start();
    cerberus::thread::Thread::sleep(10);  // sleep THIS thread
    //
    auto socket = TCPSocket("TCP transmitter");
    logDebug("connecting..");
    ASSERT_EQ(socket.connect(cerberus::Host("localhost:33333")).res, cerberus::OR_OK);
    logDebug("connected");
    cerberus::data::ByteBuffer buf("Hello, World!");
    ASSERT_EQ(socket.send(buf).res, cerberus::OR_OK);
    socket.close();
    EXPECT_EQ(receiver.join().expect().i, THREAD_SUCCESS);
}

TEST(socketTest, TCP_P2P)
{
    // creating receiver thread
    cerberus::thread::Thread receiver(cerberus::TP_OneShot, "receiverTestThread");
    receiver.provideTickCallback(&testCallback_TCP_P2P);
    receiver.start();
    cerberus::thread::Thread::sleep(10);  // sleep THIS thread
    //
    auto socket = TCPP2PSocket("TCPP2P transmitter");
    ASSERT_EQ(socket.bind(cerberus::Host("localhost:57829")).res, cerberus::OR_OK);
    ASSERT_EQ(socket.connectP2P(cerberus::Host("localhost:44444"), 2000).res, cerberus::OR_OK);
    cerberus::data::ByteBuffer buf("Hello, World!");
    ASSERT_EQ(socket.send(buf).res, cerberus::OR_OK);

    socket.close();
    EXPECT_EQ(receiver.join().expect().i, THREAD_SUCCESS);
}

TEST(socketTest, FTP)
{
    // creating receiver thread
    cerberus::thread::Thread receiver(cerberus::TP_OneShot, "receiverTestThread");
    receiver.provideTickCallback(&testCallback_FTP);
    receiver.start();
    cerberus::thread::Thread::sleep(10);  // sleep THIS thread
    //
    // file creation
    cerberus::data::filesystem::File f("ftp_socket_test_file_sent.file", cerberus::FOM_ReadWriteTrunc);
    ASSERT_TRUE(f.open().ok(true));
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
    logDebug("FILE SENT");
    socket.close();
    EXPECT_EQ(receiver.join().expect().i, THREAD_SUCCESS);
    cerberus::data::filesystem::File rf("ftp_socket_test_file_received.file");
    ASSERT_TRUE(rf.open().ok(true));
    EXPECT_TRUE(rf.isEqual(f).isTrue());

    rf.close();
    f.close();
}

TEST(socketTest, TLS_google)  // this test opens a TLS socket to google.com and gets the web page
{
    auto socket = TCPSocket("TLS socket");
    ASSERT_EQ(socket.TLS_init().res, cerberus::OR_OK);  // mark the socket as TLS
    socket.TLS_ignoreHangup(false).fail(true);
    logDebug("connecting..");
    cerberus::Host h("www.google.com:443");
    ASSERT_EQ(socket.connect(h).res, cerberus::OR_OK);
    logDebug("connected with encryption: PROTO: %s CIPHER: %s", socket.TLS_getProtocolName().c_str(), socket.TLS_getCipherName().c_str());
    logDebug("sending get request");
    EXPECT_EQ(socket
                  .send("GET / HTTP/1.1\r\n"
                        "Host: www.google.com\r\n"
                        //"Accept-Encoding: identity"
                        //"Content-Length: 2048"
                        "Accept-Language: en-US,en;q=0.5\r\n"
                        "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:47.0) Gecko/20100101 Firefox/47.0\r\n"
                        "Accept: text/html\r\n"
                        "Connection: keep-alive\r\n"
                        "Cache-Control: max-age=0\r\n\r\n")
                  .res,
              cerberus::OR_OK);
    cerberus::data::ByteBuffer buf;
    socket.setRecvBufferSize(8192);
    logDebug("receiving");
    auto r = socket.recv(buf, 1000, 200);

    EXPECT_TRUE(r.ok(true));

    socket.close();

    // save the content in a file
    cerberus::data::filesystem::File f("received_http_data.txt", cerberus::FOM_ReadWriteTrunc);
    f.open();
    f.write(buf);
    f.close();
}

TEST(socketTest, HTTPClient)
{
    cerberus::network::HTTPClient client("HTTP Client test");
    client.setupTLS(true);
    ASSERT_TRUE(client.connectTo("www.google.com:443").ok(true));
    logDebug("connected");
    cerberus::data::HTTPRequest req;
    req.setup(cerberus::HTTP_GET, "/", cerberus::HTTP_1_1)
        .addHeaderField("Host", "www.google.com")
        .addHeaderField("Accept-Language", "en-US,en;q=0.5")
        .addHeaderField("User-Agent", "Mozilla/5.0 (X11; Linux x86_64; rv:47.0) Gecko/20100101 Firefox/47.0")
        .addHeaderField("Accept", "text/html")
        .addHeaderField("Connection", "keep-alive")
        .addHeaderField("Cache-Control", "max-age=0");
    //
    logDebug("data to be sent:");
    logDebug(req.data().toNormalizedString().str.c_str());
    //
    EXPECT_TRUE(client.makeRequest(req).ok(true));
    logDebug("request sent");
    cerberus::data::HTTPResponse res;
    EXPECT_TRUE(client.getResponse(res, 1000, 200).ok(true));

    client.disconnect();

    EXPECT_TRUE(res.isOk());

    logDebug("received header lines:");
    for (int i = 0; i < res.getHeaderSize(); i++)
    {
        logDebug("%s: %s", res.getHeaderFieldName(i).c_str(), res.getHeaderFieldValue(i).c_str());
    }

    // save the payload in a file
    cerberus::data::filesystem::File f("received_payload.txt", cerberus::FOM_ReadWriteTrunc);
    f.open();
    f.write(res.payload());
    f.close();

    logDebug("payload written on disk");
}
