#include "httpclient.h"

#include "src/cerberus.h"
#include "src/core/cerberusutils.h"
#include "src/types.h"

#include <limits>

using namespace crb;

namespace
{
OpResData<crb::SIZE> checkedHttpOffset(int64_t value, const char* field)
{
    if (value < 0 || value > std::numeric_limits<crb::SIZE>::max())
    {
        return {OR_WrongData, CerberusUtils::strPrint("%s out of range", field)};
    }

    return static_cast<crb::SIZE>(value);
}
}

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
crb::OpRes HTTPClient::getDictFromHeader(const ByteBuffer &header, Dictionary &dict)
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
crb::OpRes HTTPClient::getStatus(const ByteBuffer &statusLine, HTTPResponse &response)
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

    auto status = CerberusUtils::stringToInt(str.substr(space1 + 1, space2));
    if (status.fail()) return status;
    if (status.value < 100 || status.value > 599) return {OR_WrongData, "HTTP status code out of range"};

    response.statusCode = static_cast<uint16_t>(status.value);
    response.message    = str.substr(space2 + 1, std::string::npos);

    return OR_OK;
}
//=============================================================================
OpResData<HTTPResponse> HTTPClient::_parseResponseHeader(const ByteBuffer &buf)  // FIX AND IMPROVE THIS
                                                                                 // METHOD
{
    // fix the parsing, HTTP may have not a body

    // find the gap between header and payload
    auto res = buf.search("\r\n\r\n");
    if (res.fail())
        return {OR_WrongData, "cannot find /r/n/r/n pattern", CerberusUtils::truncStr(buf.toString(), 1000)};

    auto gapRes = checkedHttpOffset(res.value, "HTTP header separator offset");
    if (gapRes.fail()) return gapRes;
    crb::SIZE gap = gapRes.value;

    // find the status line end
    res = buf.search("\r\n");
    if (res.fail())
        return {OR_WrongData, "cannot find status line end", CerberusUtils::truncStr(buf.toString(), 1000)};

    auto sleRes = checkedHttpOffset(res.value, "HTTP status line offset");
    if (sleRes.fail()) return sleRes;
    crb::SIZE sle = sleRes.value;

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
void HTTPClient::decodeChunkedData(ByteBuffer &data)
{
    data.resetCursor();
    ByteBuffer tmp;

    while (true)
    {
        auto str = data.getLine();  // get until \r\n
        auto parsedSize = CerberusUtils::stringToInt(str, Radix::Hexadecimal);
        if (parsedSize.fail() || parsedSize.value < 0 ||
            parsedSize.value > std::numeric_limits<crb::SIZE>::max())
            break;

        LSIZE size = static_cast<LSIZE>(parsedSize.value);

        if (size == 0) break;

        tmp.append(data.read(size));
        data.seek(data.pos() + 2);
    }

    data.resetCursor();
    data.assign(tmp);
}
//=============================================================================
HTTPClient::HTTPClient(size_t bufsize)
    : Socket(Socket_TCP),
      m_persistent(false),
      m_remote()
{
    setRecvBufferSize(bufsize);
}
//=============================================================================
HTTPClient::~HTTPClient() {}
//=============================================================================
void HTTPClient::setRemote(const Host &host) { m_remote = host; }
//=============================================================================
void HTTPClient::persistent(bool persistent) { m_persistent = persistent; }
//=============================================================================
crb::OpRes HTTPClient::makeRequest(const HTTPRequest &request)
{
    if (!Socket::isConnected())
    {
        auto r = _connect();
        if (r.fail()) return r;
    }

    m_currentStreamResponse.clear();

    auto ret = Socket::send(request.data());

    if (ret.fail()) close();

    return ret;
}
//=============================================================================
crb::OpResData<HTTPResponse> HTTPClient::getResponse(const TimeFrame &timeout, const TimeFrame &cyc)
{
    if (!Socket::isConnected()) return OR_BadConditions;

    ByteBuffer buf;

    {
        SocketCloser closer;
        if (!m_persistent) closer.assignSocket(this);

        auto res = Socket::recv_cyc(buf, timeout, cyc);
        condret(res);
    }

    return _parseResponseHeader(buf);
}
//=============================================================================
crb::OpResData<HTTPResponse> HTTPClient::getStream(const TimeFrame &timeout)
{
    if (!m_persistent) return OR_Unavailable;
    if (!Socket::isConnected()) return OR_BadConditions;

    ByteBuffer buf;

    if (m_currentStreamResponse.isNull())  // no header yet
    {
        condret(Socket::recv(buf, timeout));

        auto respHead = _parseResponseHeader(buf);

        while (respHead.res == OR_NotEnoughData && buf.size() < 1024 * 32)  // 32K buffer is the limit
        {
            ByteBuffer buf2;
            condret(Socket::recv(buf, timeout));
            buf += buf2;
            respHead = _parseResponseHeader(buf);
        }

        condret(respHead);

        m_currentStreamResponse = respHead.value;
        return m_currentStreamResponse;
    }

    // header already got, so now the stream data are received

    condret(Socket::recv(buf, timeout));  // receive just a block of data

    // return the original response header information with payload updated
    m_currentStreamResponse.setPayload(buf);
    return m_currentStreamResponse;
}
//=============================================================================
crb::OpResData<HTTPResponse> HTTPClient::get(const HTTPRequest &request, const TimeFrame &timeout,
                                                  const TimeFrame &cycTimeout)
{
    auto r = makeRequest(request);
    if (r.fail()) return r;
    return getResponse(timeout, cycTimeout);
}
//=============================================================================
OpRes HTTPClient::disconnect() { return Socket::close(); }
//=============================================================================
