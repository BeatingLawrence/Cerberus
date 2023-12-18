#include <cerberus/cerberus.h>
#include <cerberus/network/httpclient.h>
#include <cerberus/network/socket.h>
#include <cerberus/thread/thread.h>
#include <gtest/gtest.h>

#include "cerberus/data/filesystem/file.h"

#define THREAD_ERROR 1
#define THREAD_SUCCESS 304050

static int testCallback_UDP(cerberus::message::cerberus_message msg, cerberus::thread::Thread* thread)
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

static int testCallback_TCP(cerberus::message::cerberus_message msg, cerberus::thread::Thread* thread)
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

static int testCallback_TCP_P2P(cerberus::message::cerberus_message msg, cerberus::thread::Thread* thread)
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

static int testCallback_FTP(cerberus::message::cerberus_message msg, cerberus::thread::Thread* thread)
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
    cerberus::thread::Thread receiver(cerberus::thread::Thread::TP_OneShot, "receiverTestThread");
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
    cerberus::thread::Thread receiver(cerberus::thread::Thread::TP_OneShot, "receiverTestThread");
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
    cerberus::thread::Thread receiver(cerberus::thread::Thread::TP_OneShot, "receiverTestThread");
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
    cerberus::thread::Thread receiver(cerberus::thread::Thread::TP_OneShot, "receiverTestThread");
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
    client.useTLS();
    ASSERT_TRUE(client.connectTo("www.google.com:443").ok(true));
    logDebug("connected");
    cerberus::data::HTTPData data;
    data.setRequest({cerberus::data::HTTP_GET, "/", cerberus::data::HTTP_1_1});
    data.addHeaderField("Host", "www.google.com");
    data.addHeaderField("Accept-Language", "en-US,en;q=0.5");
    data.addHeaderField("User-Agent", "Mozilla/5.0 (X11; Linux x86_64; rv:47.0) Gecko/20100101 Firefox/47.0");
    data.addHeaderField("Accept", "text/html");
    data.addHeaderField("Connection", "keep-alive");
    data.addHeaderField("Cache-Control", "max-age=0");
    //
    logDebug("data to be sent:");
    logDebug(data.getData().toNormalizedString().c_str());
    //
    EXPECT_TRUE(client.makeRequest(data).ok(true));
    logDebug("request sent");
    EXPECT_TRUE(client.getResponse(data, 1000, 200).ok(true));

    client.disconnect();

    EXPECT_EQ(data.getStatus().statusCode, 200);  // 200 OK

    logDebug("received header:");
    for (int i = 0; i < data.getHeaderSize(); i++)
    {
        logDebug("%s: %s", data.getHeaderFieldName(i).c_str(), data.getHeaderFieldValue(i).c_str());
    }

    // save the payload in a file
    cerberus::data::filesystem::File f("received_payload.txt", cerberus::FOM_ReadWriteTrunc);
    f.open();
    f.write(data.getPayload());
    f.close();

    logDebug("payload written on disk");
}

#define BOT_TOKEN "6612599694:AAG2zBEEBYViDPTQZDyhIu3bbMa4aRVxSTI"

TEST(socketTest, TelegramBot)
{
    return;  // disable test

    cerberus::network::HTTPClient client("HTTP Bot Client");
    client.useTLS();
    cerberus::Host h("api.telegram.org:443");
    EXPECT_EQ(client.connectTo(h).res, cerberus::OR_OK);
    logDebug("connected!");
    cerberus::data::HTTPData data;
    data.setRequest({cerberus::data::HTTP_GET, "/bot" BOT_TOKEN "/getMe", cerberus::data::HTTP_1_1});
    data.addHeaderField("Host", "api.telegram.org");
    // data.addHeaderField("Connection", "keep-alive");
    EXPECT_FALSE(client.makeRequest(data).fail(true));
    logDebug("request sent");
    data.clear();
    EXPECT_FALSE(client.getResponse(data, 1000).fail(true));

    client.disconnect();
    EXPECT_EQ(data.getStatus().statusCode, 200);  // 200 OK

    logDebug("received header:");
    for (int i = 0; i < data.getHeaderSize(); i++)
    {
        logDebug("%s: %s", data.getHeaderFieldName(i).c_str(), data.getHeaderFieldValue(i).c_str());
    }

    // save the payload in a file
    cerberus::data::filesystem::File f("received_payload_telegram.txt", cerberus::FOM_ReadWriteTrunc);
    f.open();
    f.write(data.getPayload());
    f.close();

    logDebug("payload written on disk");
}

TEST(socketTest, TelegramBotSendMessage)
{
    return;  // disable test

    cerberus::network::HTTPClient client("HTTP Bot Client");
    client.useTLS();
    cerberus::Host h("api.telegram.org:443");
    EXPECT_EQ(client.connectTo(h).res, cerberus::OR_OK);
    logDebug("connected!");
    cerberus::data::HTTPData data;
    data.setRequest({cerberus::data::HTTP_POST, "/bot" BOT_TOKEN "/sendMessage", cerberus::data::HTTP_1_1});
    data.addHeaderField("Host", "api.telegram.org");
    data.addHeaderField("Content-Type", "application/json");
    data.addHeaderField("Content-Length", "103");
    // data.addHeaderField("Connection", "keep-alive");
    cerberus::data::ByteBuffer payload("{\"chat_id\":-1001964113365,\"text\":\"Signore e signori, questo messaggio e' stato inviato con codice C++\"}");
    data.setPayload(payload);
    EXPECT_FALSE(client.makeRequest(data).fail(true));
    logDebug("request sent");
    data.clear();
    EXPECT_FALSE(client.getResponse(data, 30000, 200).fail(true));

    client.disconnect();
    EXPECT_EQ(data.getStatus().statusCode, 200);  // 200 OK

    logDebug("received header:");
    for (int i = 0; i < data.getHeaderSize(); i++)
    {
        logDebug("%s: %s", data.getHeaderFieldName(i).c_str(), data.getHeaderFieldValue(i).c_str());
    }

    // save the payload in a file
    cerberus::data::filesystem::File f("received_payload_telegram3.txt", cerberus::FOM_ReadWriteTrunc);
    f.open();
    f.write(data.getPayload());
    f.close();

    logDebug("payload written on disk");
}
