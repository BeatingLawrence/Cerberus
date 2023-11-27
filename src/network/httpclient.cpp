#include "httpclient.h"

#include <regex>

#include "src/core/cerberusutils.h"

using namespace cerberus::network;

//=============================================================================
HTTPClient::DictResult HTTPClient::getDictFromHeader(const data::ByteBuffer &header)
{
    // debug("%s", header.toNormalizedString().c_str());

    Dictionary ret;

    header.resetCursor();

    while (true)
    {
        auto str = header.getLine();

        // debug("parsing line(%u): %s", str.length(), str.c_str());

        if (str.empty())
        {
            return {header.end() ? OR_OK : OR_Failure, ret};
        }

        auto p = str.find(": ");

        if (p == std::string::npos)
        {
            cdebug("Broken line found in the HTTP header");
            cdebug("%s", str.c_str());
            return {OR_Failure, ret};
        }

        auto key = str.substr(0, p);
        auto val = str.substr(p + 2, std::string::npos);

        ret.push_back({key, val});
    }
}
//=============================================================================
HTTPClient::StatusResult HTTPClient::getStatus(const data::ByteBuffer &statusLine)
{
    auto str = statusLine.toString();
    core::CerberusUtils::removeBlankAfter(str);
    data::HTTPStatus ret{};  // version, statuscode, message

    auto space1 = str.find_first_of(' ');
    auto space2 = str.find_first_of(' ', space1 + 1);

    std::string ver = str.substr(0, space1);
    core::CerberusUtils::toUpper(ver);

    if (core::CerberusUtils::areEqual(ver, "HTTP/1.0"))
        ret.version = data::HTTP_1_0;
    else if (core::CerberusUtils::areEqual(ver, "HTTP/1.1"))
        ret.version = data::HTTP_1_1;
    else if (core::CerberusUtils::areEqual(ver, "HTTP/2"))
        ret.version = data::HTTP_2;
    else
    {
        return StatusResult{OR_Failure, ret};
    }

    ret.statusCode = core::CerberusUtils::stringToInt(str.substr(space1 + 1, space2));
    ret.message    = str.substr(space2 + 1, std::string::npos);

    return StatusResult{OR_OK, ret};
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

        tmp.append(data.subBuffer_seek(size));
        data.seek(data.pos() + 2);
    }

    data.resetCursor();
    data.assign(tmp);
}
//=============================================================================
HTTPClient::HTTPClient(const std::string &name)
    : m_name(name),
      m_socket(nullptr),
      m_useTLS(false),
      m_certFile(),
      m_keyFile()
{
}
//=============================================================================
HTTPClient::~HTTPClient() { disconnect(); }
//=============================================================================
void HTTPClient::useTLS(bool use, const std::string &certfile, const std::string &keyfile)
{
    m_useTLS   = use;
    m_certFile = certfile;
    m_keyFile  = keyfile;
}
//=============================================================================
cerberus::OperationResult HTTPClient::connectTo(const Host &host)
{
    if (m_socket)
    {
        disconnect();
    }

    m_socket = new Socket(CerberusObject::Socket_TCP, core::CerberusUtils::strPrint("Socket of \"%s\"", m_name.c_str()));

    if (m_useTLS)
    {
        auto res = m_socket->TLS_init(m_certFile, m_keyFile);

        if (res.fail())
        {
            disconnect();
            return OR_Failure;
        }

        m_socket->TLS_ignoreHangup(false);
    }

    m_socket->setRecvBufferSize(8192);

    auto res = m_socket->connect(host);

    if (res.fail())
    {
        disconnect();
        return res;
    }

    return OR_OK;
}
//=============================================================================
void HTTPClient::disconnect()
{
    if (m_socket)
    {
        m_socket->close();
        delete m_socket;
        m_socket = nullptr;
    }
}
//=============================================================================
cerberus::OperationResult HTTPClient::makeRequest(const data::HTTPData &data)
{
    if (!m_socket)
    {
        return OR_BadConditions;
    }

    return m_socket->send(data.getData());
}
//=============================================================================
cerberus::OperationResult HTTPClient::getResponse(data::HTTPData &data, const time::TimeFrame &timeout, const time::TimeFrame &cycTimeout)
{
    if (!m_socket)
    {
        return OR_BadConditions;
    }

    data.clear();

    data::ByteBuffer buf;
    OperationResult res;

    res = m_socket->recv(buf, timeout, cycTimeout);

    if (res.fail())
    {
        return res;
    }

    res = buf.search("\r\n\r\n");  // find the gap between header and payload
    if (res.fail()) return OR_Failure;
    SIZE gap = res.i;

    res = buf.search("\r\n");  // find the status line end
    if (res.fail()) return OR_Failure;
    SIZE sle = res.i;

    data.setPayload(buf.subBuffer(gap + 4));

    // get status line
    auto gs = getStatus(buf.subBuffer(0, sle));
    if (gs.result.fail()) return OR_Failure;

    data.setStatus(gs.status);

    // get header
    auto dfe = getDictFromHeader(buf.subBuffer(sle + 2, gap + 2 - sle));

    if (dfe.result.fail()) return OR_Failure;

    for (auto &&el : dfe.dict)
    {
        data.addHeaderField(el.key, el.val);
    }
    res = data.getHeaderMatch("transfer-encoding", "chunked");

    if (res.ok() && res.i)
    {
        // chunked encoding
        cdebug("processing chunked data...");
        decodeChunkedData(data.getPayload());
    }

    return OR_OK;
}
//=============================================================================
Socket *HTTPClient::getSocket() { return m_socket; }
//=============================================================================
