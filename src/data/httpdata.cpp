#include "httpdata.h"

#include "src/exception/exceptioncatalog.h"

using namespace cerberus;

//=============================================================================
data::HTTPData::HTTPData()
    : m_type(HDT_Invalid),
      m_request({}),
      m_status({})
{
}
//=============================================================================
void data::HTTPData::setRequest(const HTTPRequest &request)
{
    m_request = request;
    m_type    = HDT_Request;
}
//=============================================================================
void data::HTTPData::setStatus(const HTTPStatus &status)
{
    m_status = status;
    m_type   = HDT_Response;
}
//=============================================================================
void data::HTTPData::addHeaderField(const std::string &name, const std::string &value) { m_header.push_back({core::CerberusUtils::toLower(name), value}); }
//=============================================================================
void data::HTTPData::removeHeaderField(const std::string &name)
{
    auto str = core::CerberusUtils::toLower(name);

    for (auto it = m_header.begin(); it < m_header.end(); it++)
    {
        if (cerberus::core::CerberusUtils::areEqual((*it).key, str))
        {
            m_header.erase(it);
            return;
        }
    }
}
//=============================================================================
void data::HTTPData::setPayload(const data::ByteBuffer &payload) { m_payload = payload; }
//=============================================================================
void data::HTTPData::clear()
{
    m_type    = HDT_Invalid;
    m_request = {};
    m_status  = {};
    m_header.clear();
    m_payload.clear();
}
//=============================================================================
bool data::HTTPData::isValid() const { return m_type == HDT_Invalid; }
//=============================================================================
bool data::HTTPData::isRequest() const { return m_type == HDT_Request; }
//=============================================================================
bool data::HTTPData::isResponse() const { return m_type == HDT_Response; }
//=============================================================================
data::HTTPRequest data::HTTPData::getRequest() const { return m_request; }
//=============================================================================
data::HTTPStatus data::HTTPData::getStatus() const { return m_status; }
//=============================================================================
cerberus::SIZE data::HTTPData::getHeaderSize() const { return m_header.size(); }
//=============================================================================
cerberus::OperationResult data::HTTPData::getHeaderField(const std::string &key) const
{
    auto str = core::CerberusUtils::toLower(key);

    for (auto it = m_header.begin(); it < m_header.end(); it++)
    {
        if (cerberus::core::CerberusUtils::areEqual((*it).key, str))
        {
            return (*it).val;
        }
    }
    return OR_NotFound;
}
//=============================================================================
OperationResult data::HTTPData::getHeaderMatch(const std::string &key, const std::string &value) const
{
    auto res = getHeaderField(key);

    if (res.fail())
    {
        return res;
    }

    return (int64_t)(core::CerberusUtils::areEqual(value, res.str));
}
//=============================================================================
std::string data::HTTPData::getHeaderFieldName(SIZE index) const
{
    if (index >= getHeaderSize()) throw cerberusIllegalArgExc("index out of range");

    return m_header[index].key;
}
//=============================================================================
std::string data::HTTPData::getHeaderFieldValue(SIZE index) const
{
    if (index >= getHeaderSize()) throw cerberusIllegalArgExc("index out of range");

    return m_header[index].val;
}
//=============================================================================
data::ByteBuffer data::HTTPData::getHeader() const
{
    data::ByteBuffer buf;

    switch (getRequest().method)
    {
        case data::HTTP_GET:
            buf.appendString("GET ");
            break;
        case data::HTTP_POST:
            buf.appendString("POST ");
            break;
        case data::HTTP_HEAD:
            buf.appendString("HEAD ");
            break;
        case data::HTTP_PUT:
            buf.appendString("PUT ");
            break;
        case data::HTTP_DELETE:
            buf.appendString("DELETE ");
            break;
        case data::HTTP_PATCH:
            buf.appendString("PATCH ");
            break;
        case data::HTTP_TRACE:
            buf.appendString("TRACE ");
            break;
        case data::HTTP_OPTIONS:
            buf.appendString("OPTIONS ");
            break;
        case data::HTTP_CONNECT:
            buf.appendString("CONNECT ");
            break;
    }

    buf.appendString(getRequest().url.c_str());
    buf.appendString(" ");

    switch (getRequest().version)
    {
        case data::HTTP_1_0:
            buf.appendString("HTTP/1.0\r\n");
            break;
        case data::HTTP_1_1:
            buf.appendString("HTTP/1.1\r\n");
            break;
        case data::HTTP_2:
            buf.appendString("HTTP/2\r\n");
            break;
    }

    for (SIZE i = 0; i < getHeaderSize(); i++)
    {
        buf.appendString(core::CerberusUtils::strPrint("%s: %s\r\n", getHeaderFieldName(i).c_str(), getHeaderFieldValue(i).c_str()).c_str());
    }

    buf.append("\r\n");

    return buf;
}
//=============================================================================
data::ByteBuffer data::HTTPData::getData() const
{
    data::ByteBuffer buf = getHeader();
    buf.append(getPayload());
    return buf;
}
//=============================================================================
const cerberus::data::ByteBuffer &data::HTTPData::getPayload() const { return m_payload; }
//=============================================================================
data::ByteBuffer &data::HTTPData::getPayload() { return m_payload; }
//=============================================================================
