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
            debug("Broken line found in the HTTP header");
            debug("%s", str.c_str());
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
        ret.version = data::HV_1_0;
    else if (core::CerberusUtils::areEqual(ver, "HTTP/1.1"))
        ret.version = data::HV_1_1;
    else if (core::CerberusUtils::areEqual(ver, "HTTP/2"))
        ret.version = data::HV_2;
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
void HTTPClient::useTLS(bool use, const std::string &certfile, const std::string &keyfile)
{
    m_useTLS   = use;
    m_certFile = certfile;
    m_keyFile  = keyfile;
}
//=============================================================================
cerberus::OperationResult HTTPClient::connectTo(const Host &host)
{
    if (!m_socket)
    {
        m_socket = new Socket(CerberusObject::Socket_TCP, core::CerberusUtils::strPrint("[HTTP Client]", m_name.c_str()));
    }

    if (m_useTLS)
    {
        m_socket->TLS_init(m_certFile, m_keyFile);
    }

    return m_socket->connect(host);
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

    data::ByteBuffer buf;

    switch (data.getRequest().method)
    {
        case data::HM_GET:
            buf.appendString("GET ");
            break;
        case data::HM_POST:
            buf.appendString("POST ");
            break;
        case data::HM_HEAD:
            buf.appendString("HEAD ");
            break;
        case data::HM_PUT:
            buf.appendString("PUT ");
            break;
        case data::HM_DELETE:
            buf.appendString("DELETE ");
            break;
        case data::HM_PATCH:
            buf.appendString("PATCH ");
            break;
        case data::HM_TRACE:
            buf.appendString("TRACE ");
            break;
        case data::HM_OPTIONS:
            buf.appendString("OPTIONS ");
            break;
        case data::HM_CONNECT:
            buf.appendString("CONNECT ");
            break;
    }

    buf.appendString(data.getRequest().url.c_str());
    buf.appendString(" ");

    switch (data.getRequest().version)
    {
        case data::HV_1_0:
            buf.appendString("HTTP/1.0\r\n");
            break;
        case data::HV_1_1:
            buf.appendString("HTTP/1.1\r\n");
            break;
        case data::HV_2:
            buf.appendString("HTTP/2\r\n");
            break;
    }

    for (SIZE i = 0; i < data.getHeaderSize(); i++)
    {
        buf.appendString(core::CerberusUtils::strPrint("%s: %s\r\n", data.getHeaderFieldName(i).c_str(), data.getHeaderFieldValue(i).c_str()).c_str());
    }

    buf.append("\r\n");
    buf.append(data.getPayload());  //=============================================================================

    return m_socket->send(buf);
}
//=============================================================================
cerberus::OperationResult HTTPClient::getResponse(data::HTTPData &data, const time::Time &timeout)
{
    if (!m_socket)
    {
        return OR_BadConditions;
    }

    data::ByteBuffer buf;
    OperationResult res;

    res = m_socket->recv(buf, timeout);

    if (res.fail())
    {
        return res;
    }

    res = buf.search("\r\n\r\n");  // find the gap between header and payload
    if (res.fail()) return OR_Failure;
    SIZE gap = res.sz;

    res = buf.search("\r\n");  // find the status line end
    if (res.fail()) return OR_Failure;
    SIZE sle = res.sz;

    data.clear();
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

    if (res.ok() && res.b1)
    {
        // chunked encoding
        debug("processing chunked data...");
        decodeChunkedData(data.getPayload());
    }

    return OR_OK;
}
//=============================================================================
