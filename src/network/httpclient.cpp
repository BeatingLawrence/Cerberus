#include "httpclient.h"

#include "src/cerberus.h"
#include "src/core/cerberusutils.h"

using namespace cerberus;

//=============================================================================
OpRes HTTPClient::_connect()
{
    auto res = Socket::reset();

    if (res.fail()) return res;

    res = Socket::connect(m_remote);

    if (res.fail()) return res;

    return OR_OK;
}
//=============================================================================
cerberus::OpRes HTTPClient::getDictFromHeader(const ByteBuffer &header, Dictionary &dict)
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
cerberus::OpRes HTTPClient::getStatus(const ByteBuffer &statusLine, HTTPResponse &response)
{
    auto str = statusLine.toString();
    CerberusUtils::removeBlankAfter(str);

    auto space1 = str.find_first_of(' ');
    auto space2 = str.find_first_of(' ', space1 + 1);

    std::string ver = str.substr(0, space1);
    CerberusUtils::toUpper(ver);

    if (CerberusUtils::areEqual(ver, "HTTP/1.0"))
        response.version = HTTP_1_0;
    else if (CerberusUtils::areEqual(ver, "HTTP/1.1"))
        response.version = HTTP_1_1;
    else if (CerberusUtils::areEqual(ver, "HTTP/2"))
        response.version = HTTP_2;
    else
    {
        return {OR_Failure, "Unrecognized HTTP version received"};
    }

    response.statusCode = CerberusUtils::stringToInt(str.substr(space1 + 1, space2)).value;
    response.message    = str.substr(space2 + 1, std::string::npos);

    return OR_OK;
}
//=============================================================================
void HTTPClient::decodeChunkedData(ByteBuffer &data)
{
    data.resetCursor();
    ByteBuffer tmp;

    while (true)
    {
        auto str = data.getLine();  // get until \r\n
        int size = CerberusUtils::stringToInt(str, Radix::Hexadecimal).value;

        if (size == 0) break;

        tmp.append(data.read(size));
        data.seek(data.pos() + 2);
    }

    data.resetCursor();
    data.assign(tmp);
}
//=============================================================================
HTTPClient::HTTPClient()
    : Socket(Socket::Socket_TCP),
      m_persistent(false),
      m_remote()
{
    setRecvBufferSize(8192);  // 8K buffer size
}
//=============================================================================
HTTPClient::~HTTPClient() {}
//=============================================================================
void HTTPClient::setRemote(const Host &host) { m_remote = host; }
//=============================================================================
void HTTPClient::persistent(bool persistent) { m_persistent = persistent; }
//=============================================================================
cerberus::OpRes HTTPClient::makeRequest(const HTTPRequest &request)
{
    if (!Socket::isConnected())
    {
        auto r = _connect();
        if (r.fail()) return r;
    }

    auto ret = Socket::send(request.data());

    if (ret.fail()) close();

    return ret;
}
//=============================================================================
cerberus::OpResData<HTTPResponse> HTTPClient::getResponse(const TimeFrame &timeout,
                                                          const TimeFrame &cycTimeout)
{
    if (!Socket::isConnected()) return OR_BadConditions;

    SocketCloser closer;
    if (!m_persistent) closer.assignSocket(this);

    ByteBuffer buf;

    {
        auto res = Socket::recv(buf, timeout, cycTimeout);
        if (res.fail())
        {
            if (res.res != OR_Hangup || buf.isEmpty()) return res;
        }
    }

    // fix the parsing, HTTP may have not a body

    // find the gap between header and payload
    auto res = buf.search("\r\n\r\n");
    if (res.fail()) return {OR_WrongData, CerberusUtils::truncStr(buf.toString(), 1000)};

    SIZE gap = res.value;

    // find the status line end
    res = buf.search("\r\n");
    if (res.fail()) return {OR_WrongData, CerberusUtils::truncStr(buf.toString(), 1000)};

    SIZE sle = res.value;

    HTTPResponse data;
    data.setPayload(buf.subBuffer(gap + 4));

    // get status line
    res = getStatus(buf.subBuffer(0, sle), data);
    if (res.fail()) return res;

    // get header
    Dictionary dict;

    if (getDictFromHeader(buf.subBuffer(sle + 2, gap + 2 - sle), dict).fail())
        return {OR_WrongData, CerberusUtils::truncStr(buf.toString(), 1000)};

    data.setHeaderDict(dict);

    if (data.getHeaderMatch("transfer-encoding", "chunked").ok())
    {
        // chunked encoding
        decodeChunkedData(data.payload());
    }

    return data;
}
//=============================================================================
cerberus::OpResData<HTTPResponse> HTTPClient::get(const HTTPRequest &request, const TimeFrame &timeout,
                                                  const TimeFrame &cycTimeout)
{
    auto r = makeRequest(request);
    if (r.fail()) return r;
    return getResponse(timeout, cycTimeout);
}
//=============================================================================
