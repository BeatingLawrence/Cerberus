#include "httpclient.h"

#include "src/cerberus.h"
#include "src/core/cerberusutils.h"

using namespace cerberus::network;
using namespace cerberus::core;

//=============================================================================
cerberus::OperationResult HTTPClient::getDictFromHeader(const data::ByteBuffer &header, Dictionary &dict)
{
    // debug("%s", header.toNormalizedString().c_str());

    header.resetCursor();
    dict.clear();

    OperationResult failed(OR_Failure, "Broken line found in HTTP header");

    while (true)
    {
        auto str = header.getLine();

        // debug("parsing line(%u): %s", str.length(), str.c_str());

        if (str.empty())
        {
            return header.isEnd() ? OR_OK : failed;
        }

        auto p = str.find(": ");

        if (p == std::string::npos)
        {
            failed.str.append("\n");
            failed.str.append(str);
            return failed;
        }

        auto key = str.substr(0, p);
        auto val = str.substr(p + 2, std::string::npos);

        dict.push_back({key, val});
    }
}
//=============================================================================
cerberus::OperationResult HTTPClient::getStatus(const data::ByteBuffer &statusLine, data::HTTPResponse &response)
{
    auto str = statusLine.toString();
    core::CerberusUtils::removeBlankAfter(str);

    auto space1 = str.find_first_of(' ');
    auto space2 = str.find_first_of(' ', space1 + 1);

    std::string ver = str.substr(0, space1);
    core::CerberusUtils::toUpper(ver);

    if (core::CerberusUtils::areEqual(ver, "HTTP/1.0"))
        response.version = HTTP_1_0;
    else if (core::CerberusUtils::areEqual(ver, "HTTP/1.1"))
        response.version = HTTP_1_1;
    else if (core::CerberusUtils::areEqual(ver, "HTTP/2"))
        response.version = HTTP_2;
    else
    {
        return {OR_Failure, "Unrecognized HTTP version received"};
    }

    response.statusCode = core::CerberusUtils::stringToInt(str.substr(space1 + 1, space2));
    response.message    = str.substr(space2 + 1, std::string::npos);

    return OR_OK;
}
//=============================================================================
void HTTPClient::decodeChunkedData(data::ByteBuffer &data)
{
    data.resetCursor();
    data::ByteBuffer tmp;

    while (true)
    {
        auto str = data.getLine();  // get until \r\n
        int size = cerberus::core::CerberusUtils::stringToInt(str, Radix::Hexadecimal);

        if (size == 0)
        {
            break;
        }

        tmp.append(data.read(size));
        data.seek(data.pos() + 2);
    }

    data.resetCursor();
    data.assign(tmp);
}
//=============================================================================
HTTPClient::HTTPClient(const std::string &name)
    : m_socket(CerberusObject::Socket_TCP, core::CerberusUtils::strPrint("Socket of \"%s\"", name.c_str()))
{
    m_socket.setRecvBufferSize(4096);  // 4K buffer size
}
//=============================================================================
HTTPClient::~HTTPClient() { disconnect(); }
//=============================================================================
void HTTPClient::setupTLS(bool use, const std::string &certfile, const std::string &keyfile)
{
    if (!use)
    {
        m_socket.TLS_deinit();
        return;
    }

    m_socket.TLS_init(certfile, keyfile);
    m_socket.TLS_ignoreHangup();
}
//=============================================================================
cerberus::OperationResult HTTPClient::connectTo(const Host &host)
{
    auto res = m_socket.reset();

    if (res.fail()) return res;

    res = m_socket.connect(host);

    if (res.fail()) return res;

    return OR_OK;
}
//=============================================================================
void HTTPClient::disconnect() { m_socket.close(); }
//=============================================================================
cerberus::OperationResult HTTPClient::makeRequest(const data::HTTPRequest &data)
{
    if (!m_socket.isConnected())
    {
        return OR_BadConditions;
    }

    return m_socket.send(data.data());
}
//=============================================================================
cerberus::OperationResult HTTPClient::getResponse(data::HTTPResponse &data, const time::TimeFrame &timeout, const time::TimeFrame &cycTimeout)
{
    if (!m_socket.isConnected()) return OR_BadConditions;

    data.clear();

    data::ByteBuffer buf;
    OperationResult res;

    res = m_socket.recv(buf, timeout, cycTimeout);

    if (res.fail()) return res;

    res = buf.search("\r\n\r\n");  // find the gap between header and payload
    if (res.fail()) return res;
    SIZE gap = res.i;

    res = buf.search("\r\n");  // find the status line end
    if (res.fail()) return res;
    SIZE sle = res.i;

    data.setPayload(buf.subBuffer(gap + 4));

    // get status line
    res = getStatus(buf.subBuffer(0, sle), data);
    if (res.fail()) return res;

    // get header
    Dictionary dict;
    res = getDictFromHeader(buf.subBuffer(sle + 2, gap + 2 - sle), dict);

    if (res.fail()) return OR_Failure;

    for (auto &&el : dict)
    {
        data.addHeaderField(el.key, el.val);
    }

    res = data.getHeaderMatch("transfer-encoding", "chunked");

    if (res.ok() && res.i)
    {
        // chunked encoding
        logDebug("processing chunked data...");
        decodeChunkedData(data.payload());
    }

    return OR_OK;
}
//=============================================================================
Socket *HTTPClient::getSocket() { return &m_socket; }
//=============================================================================
