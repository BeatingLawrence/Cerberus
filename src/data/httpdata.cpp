#include "httpdata.h"

#include "src/exception/exception.h"

using namespace cerberus;

//=============================================================================
data::HTTPData::HTTPData()
    : m_header(),
      m_payload()
{
}
//=============================================================================
data::HTTPData &data::HTTPData::addHeaderField(const std::string &name, const std::string &value)
{
    m_header.push_back({core::CerberusUtils::toLower(name), value});
    return *this;
}
//=============================================================================
data::HTTPData &data::HTTPData::setHeaderDict(const Dictionary &dict)
{
    m_header = dict;
    return *this;
}
//=============================================================================
data::HTTPData &data::HTTPData::removeHeaderField(const std::string &name)
{
    auto str = core::CerberusUtils::toLower(name);

    for (auto it = m_header.begin(); it < m_header.end(); it++)
    {
        if (cerberus::core::CerberusUtils::areEqual((*it).key, str))
        {
            m_header.erase(it);
            break;
        }
    }

    return *this;
}
//=============================================================================
data::HTTPData &data::HTTPData::setPayload(const data::ByteBuffer &payload)
{
    m_payload = payload;
    return *this;
}
//=============================================================================
const cerberus::data::ByteBuffer &data::HTTPData::payload() const { return m_payload; }
//=============================================================================
data::ByteBuffer &data::HTTPData::payload() { return m_payload; }
//=============================================================================
data::HTTPData &data::HTTPData::clear()
{
    m_header.clear();
    m_payload.clear();
    return *this;
}
//=============================================================================
data::HTTPData &data::HTTPData::clearHeader()
{
    m_header.clear();
    return *this;
}
//=============================================================================
cerberus::SIZE data::HTTPData::getHeaderSize() const { return m_header.size(); }
//=============================================================================
cerberus::OperationResult data::HTTPData::getHeaderField(const std::string &key) const { return m_header.getFieldValue(key, WM_CaseInsensitive); }
//=============================================================================
OperationResult data::HTTPData::getHeaderMatch(const std::string &key, const std::string &value) const { return m_header.getFieldMatch(key, value, WM_CaseInsensitive, WM_CaseSensitive); }
//=============================================================================
std::string data::HTTPData::getHeaderFieldName(SIZE index) const { return m_header.getNameAt(index); }
//=============================================================================
std::string data::HTTPData::getHeaderFieldValue(SIZE index) const { return m_header.getValueAt(index); }
//=============================================================================
data::HTTPRequest::HTTPRequest()
    : method(HTTP_GET),
      url("/"),
      version(HTTP_1_1)
{
}
//=============================================================================
data::HTTPRequest &data::HTTPRequest::setup(HTTPMethod m, const std::string &u, HTTPVersion v)
{
    method  = m;
    url     = u;
    version = v;
    return *this;
}
//=============================================================================
data::ByteBuffer data::HTTPRequest::data() const
{
    data::ByteBuffer buf;

    switch (method)
    {
        case HTTP_GET:
            buf.appendString("GET ");
            break;
        case HTTP_POST:
            buf.appendString("POST ");
            break;
        case HTTP_HEAD:
            buf.appendString("HEAD ");
            break;
        case HTTP_PUT:
            buf.appendString("PUT ");
            break;
        case HTTP_DELETE:
            buf.appendString("DELETE ");
            break;
        case HTTP_PATCH:
            buf.appendString("PATCH ");
            break;
        case HTTP_TRACE:
            buf.appendString("TRACE ");
            break;
        case HTTP_OPTIONS:
            buf.appendString("OPTIONS ");
            break;
        case HTTP_CONNECT:
            buf.appendString("CONNECT ");
            break;
    }

    buf.appendString(url.c_str());
    buf.appendChar(' ');

    switch (version)
    {
        case HTTP_1_0:
            buf.appendString("HTTP/1.0\r\n");
            break;
        case HTTP_1_1:
            buf.appendString("HTTP/1.1\r\n");
            break;
        case HTTP_2:
            buf.appendString("HTTP/2\r\n");
            break;
    }

    for (SIZE i = 0; i < getHeaderSize(); i++)
    {
        buf.appendString(core::CerberusUtils::strPrint("%s: %s\r\n", getHeaderFieldName(i).c_str(), getHeaderFieldValue(i).c_str()).c_str());
    }

    buf.appendString("\r\n");
    buf.append(payload());

    return buf;
}
//=============================================================================
data::HTTPResponse::HTTPResponse()
    : version(HTTP_1_1),
      statusCode(0),
      message("UNDEFINED")

{
}
//=============================================================================
data::HTTPResponse &data::HTTPResponse::setup(HTTPVersion v, uint16_t sc, const std::string &msg)
{
    version    = v;
    statusCode = sc;
    message    = msg;
    return *this;
}
//=============================================================================
data::ByteBuffer data::HTTPResponse::data() const
{
    data::ByteBuffer buf;

    switch (version)
    {
        case HTTP_1_0:
            buf.appendString("HTTP/1.0");
            break;
        case HTTP_1_1:
            buf.appendString("HTTP/1.1");
            break;
        case HTTP_2:
            buf.appendString("HTTP/2");
            break;
    }

    buf.appendString(core::CerberusUtils::strPrint(" %u ", statusCode).c_str());
    buf.appendString(message.c_str());
    buf.append("\r\n");

    for (SIZE i = 0; i < getHeaderSize(); i++)
    {
        buf.appendString(core::CerberusUtils::strPrint("%s: %s\r\n", getHeaderFieldName(i).c_str(), getHeaderFieldValue(i).c_str()).c_str());
    }

    buf.append("\r\n");
    buf.append(payload());

    return buf;
}
//=============================================================================
bool data::HTTPResponse::isOk() { return statusCode != 0 && statusCode < 400; }
//=============================================================================
