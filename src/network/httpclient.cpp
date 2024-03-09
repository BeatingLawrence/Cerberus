#include "httpclient.h"

#include "src/cerberus.h"
#include "src/core/cerberusutils.h"

using namespace cerberus::network;
using namespace cerberus::core;

//=============================================================================
cerberus::OpRes HTTPClient::_connect()
{
    auto res = m_socket.reset();

    if (res.fail()) return res;

    res = m_socket.connect(m_server);

    if (res.fail()) return res;

    return OR_OK;
}
//=============================================================================
cerberus::OpRes HTTPClient::getDictFromHeader(const data::ByteBuffer &header, Dictionary &dict)
{
    // debug("%s", header.toNormalizedString().c_str());

    header.resetCursor();
    dict.clear();

    OpRes failed(OR_Failure, "Broken line found in HTTP header");

    while (true)
    {
        auto str = header.getLine();

        // debug("parsing line(%u): %s", str.length(), str.c_str());

        if (str.empty()) return header.isEnd() ? OR_OK : failed;

        auto p = str.find(": ");

        if (p == std::string::npos)
        {
            failed.reason.append("\n");
            failed.reason.append(str);
            return failed;
        }

        auto key = str.substr(0, p);
        auto val = str.substr(p + 2, std::string::npos);

        dict.push_back({key, val});
    }
}
//=============================================================================
cerberus::OpRes HTTPClient::getStatus(const data::ByteBuffer &statusLine, data::HTTPResponse &response)
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

    response.statusCode = core::CerberusUtils::stringToInt(str.substr(space1 + 1, space2)).value;
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
        int size = cerberus::core::CerberusUtils::stringToInt(str, Radix::Hexadecimal).value;

        if (size == 0) break;

        tmp.append(data.read(size));
        data.seek(data.pos() + 2);
    }

    data.resetCursor();
    data.assign(tmp);
}
//=============================================================================
HTTPClient::HTTPClient(const std::string &name)
    : m_socket(CerberusObject::Socket_TCP, core::CerberusUtils::strPrint("Socket of \"%s\"", name.c_str())),
      m_persistent(false),
      m_server()
{
    m_socket.setRecvBufferSize(4096);  // 4K buffer size
}
//=============================================================================
HTTPClient::~HTTPClient() { disconnect(); }
//=============================================================================
cerberus::OpRes HTTPClient::TLS_init(const std::string &ca_file, const std::string &certfile,
                                     const std::string &keyfile)
{
    return m_socket.TLS_init(certfile, keyfile, ca_file);
}
//=============================================================================
cerberus::OpRes HTTPClient::TLS_ignoreHangup(bool ignore) { return m_socket.TLS_ignoreHangup(ignore); }
//=============================================================================
cerberus::OpRes HTTPClient::TLS_deinit() { return m_socket.TLS_deinit(); }
//=============================================================================
cerberus::OpRes HTTPClient::connect(const Host &host)
{
    m_server = host;
    return _connect();
}
//=============================================================================
void HTTPClient::disconnect() { m_socket.close(); }
//=============================================================================
void HTTPClient::persistent(bool persistent) { m_persistent = persistent; }
//=============================================================================
cerberus::OpRes HTTPClient::makeRequest(const data::HTTPRequest &request)
{
    if (!m_socket.isConnected())
    {
        if (m_persistent)
        {
            auto r = _connect();
            if (r.fail()) return r;
        }
        else
            return OR_BadConditions;
    }

    return m_socket.send(request.data());
}
//=============================================================================
cerberus::OpResData<cerberus::data::HTTPResponse> HTTPClient::getResponse(const time::TimeFrame &timeout,
                                                                          const time::TimeFrame &cycTimeout)
{
    if (!m_socket.isConnected()) return OR_BadConditions;

    data::ByteBuffer buf;

    {
        auto res = m_socket.recv(buf, timeout, cycTimeout);
        if (res.fail())
        {
            if (res.res != OR_Hangup || buf.isEmpty()) return res;
        }
    }

    // fix the parsing, HTTP may have not a body

    // find the gap between header and payload
    auto res = buf.search("\r\n\r\n");
    if (res.fail()) return {OR_WrongData, core::CerberusUtils::truncStr(buf.toString(), 1000)};
    SIZE gap = res.value;

    // find the status line end
    res = buf.search("\r\n");
    if (res.fail()) return {OR_WrongData, core::CerberusUtils::truncStr(buf.toString(), 1000)};
    SIZE sle = res.value;

    data::HTTPResponse data;
    data.setPayload(buf.subBuffer(gap + 4));

    // get status line
    res = getStatus(buf.subBuffer(0, sle), data);
    if (res.fail()) return res;

    // get header
    Dictionary dict;

    if (getDictFromHeader(buf.subBuffer(sle + 2, gap + 2 - sle), dict).fail())
        return {OR_WrongData, core::CerberusUtils::truncStr(buf.toString(), 1000)};

    data.setHeaderDict(dict);

    if (data.getHeaderMatch("transfer-encoding", "chunked").ok())
    {
        // chunked encoding
        logDebug("processing chunked data...");
        decodeChunkedData(data.payload());
    }

    return data;
}
//=============================================================================
cerberus::OpResData<cerberus::data::HTTPResponse> HTTPClient::get(const data::HTTPRequest &request,
                                                                  const time::TimeFrame &timeout,
                                                                  const time::TimeFrame &cycTimeout)
{
    auto r = makeRequest(request);

    if (r.fail()) return r;

    return getResponse(timeout, cycTimeout);
}
//=============================================================================
Socket &HTTPClient::getSocket() { return m_socket; }
//=============================================================================
