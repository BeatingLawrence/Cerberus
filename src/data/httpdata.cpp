#include "httpdata.h"

#include "../core/cerberusutils.h"
#include "../exception/exception.h"

#include <limits>

using namespace crb;

static constexpr const char *contentlen = "Content-Length";

//=============================================================================
void HTTPData::setPayloadSize(crb::LSIZE s)
{
    auto str = CerberusUtils::strPrint_uint(s);

    if (m_header.exists(contentlen, WM_CaseInsensitive))
    {
        // update it
        m_header.get(contentlen, WM_CaseInsensitive).val = str;
    }
    else
    {
        // create it
        m_header.addKey(contentlen, str);
    }
}
//=============================================================================
HTTPData::HTTPData()
    : m_header(),
      m_payload()
{
}
//=============================================================================
HTTPData &HTTPData::addHeaderField(const std::string &name, const std::string &value)
{
    m_header.push_back({CerberusUtils::toLower(name), value});
    return *this;
}
//=============================================================================
HTTPData &HTTPData::setHeaderDict(const Dictionary &dict)
{
    m_header = dict;
    return *this;
}
//=============================================================================
HTTPData &HTTPData::removeHeaderField(const std::string &name)
{
    auto str = CerberusUtils::toLower(name);

    for (auto it = m_header.begin(); it < m_header.end(); it++)
    {
        if (CerberusUtils::areEqual((*it).key, str))
        {
            m_header.erase(it);
            break;
        }
    }

    return *this;
}
//=============================================================================
HTTPData &HTTPData::setPayload(const ByteBuffer &payload)
{
    m_payload = payload;
    return *this;
}
//=============================================================================
HTTPData &HTTPData::updatePayloadSizeHeader()
{
    setPayloadSize(m_payload.size());
    return *this;
}
//=============================================================================
OpRes HTTPData::setJsonPayload(const JsonData &payload)
{
    auto r = payload.generate();

    if (r.fail()) return r;

    m_payload = r.value;
    setPayloadSize(m_payload.size());
    return r;
}
//=============================================================================
const ByteBuffer &HTTPData::payload() const { return m_payload; }
//=============================================================================
ByteBuffer &HTTPData::payload() { return m_payload; }
//=============================================================================
OpResData<JsonData> HTTPData::JsonPayload() const
{
    JsonData jd;
    auto r = jd.parse(m_payload);
    if (r.fail()) return r;

    return jd;
}
//=============================================================================
HTTPData &HTTPData::clear()
{
    m_header.clear();
    m_payload.clear();
    return *this;
}
//=============================================================================
HTTPData &HTTPData::clearHeader()
{
    m_header.clear();
    return *this;
}
//=============================================================================
crb::SIZE HTTPData::getHeaderSize() const
{
    if (m_header.size() > std::numeric_limits<SIZE>::max())
    {
        throw cIllegalStateExc("HTTP header has too many fields");
    }

    return static_cast<SIZE>(m_header.size());
}
//=============================================================================
StringOpRes HTTPData::getHeaderField(const std::string &key) const
{
    return m_header.getFieldValue(key, WM_CaseInsensitive);
}
//=============================================================================
OpRes HTTPData::getHeaderMatch(const std::string &key, const std::string &value) const
{
    return m_header.getFieldMatch(key, value, WM_CaseInsensitive, WM_CaseSensitive);
}
//=============================================================================
std::string HTTPData::getHeaderFieldName(crb::SIZE index) const { return m_header.getNameAt(index); }
//=============================================================================
std::string HTTPData::getHeaderFieldValue(crb::SIZE index) const { return m_header.getValueAt(index); }
//=============================================================================
HTTPRequest::HTTPRequest()
    : method(HTTP_GET),
      url("/"),
      version(HTTP_1_1)
{
}
//=============================================================================
HTTPRequest &HTTPRequest::setup(HTTPMethod m, const std::string &u, HTTPVersion v)
{
    method  = m;
    url     = u;
    version = v;
    return *this;
}
//=============================================================================
ByteBuffer HTTPRequest::data() const
{
    ByteBuffer buf;

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

    for (crb::SIZE i = 0; i < getHeaderSize(); i++)
    {
        buf.appendString(CerberusUtils::strPrint("%s: %s\r\n", getHeaderFieldName(i).c_str(),
                                                 getHeaderFieldValue(i).c_str())
                             .c_str());
    }

    buf.appendString("\r\n");
    buf.append(payload());

    return buf;
}
//=============================================================================
HTTPResponse::HTTPResponse()
    : version(HTTP_1_1),
      statusCode(0)

{
}
//=============================================================================
HTTPResponse &HTTPResponse::setup(HTTPVersion v, uint16_t sc, const std::string &msg)
{
    version    = v;
    statusCode = sc;
    message    = msg;
    return *this;
}
//=============================================================================
ByteBuffer HTTPResponse::data() const
{
    ByteBuffer buf;

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

    buf.appendString(CerberusUtils::strPrint(" %u ", statusCode).c_str());
    buf.appendString(message.c_str());
    buf.append("\r\n");

    for (crb::SIZE i = 0; i < getHeaderSize(); i++)
    {
        buf.appendString(CerberusUtils::strPrint("%s: %s\r\n", getHeaderFieldName(i).c_str(),
                                                 getHeaderFieldValue(i).c_str())
                             .c_str());
    }

    buf.append("\r\n");
    buf.append(payload());

    return buf;
}
//=============================================================================
bool HTTPResponse::isOk() { return statusCode != 0 && statusCode < 400; }
//=============================================================================
void HTTPResponse::clear()
{
    statusCode = 0;
    message.clear();
    HTTPData::clear();
}
//=============================================================================
bool HTTPResponse::isNull() { return statusCode == 0; }
//=============================================================================
