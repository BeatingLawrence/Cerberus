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

    return (core::CerberusUtils::areEqual(value, res.str));
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
const cerberus::data::ByteBuffer &data::HTTPData::getPayload() const { return m_payload; }
//=============================================================================
data::ByteBuffer &data::HTTPData::getPayload() { return m_payload; }
//=============================================================================
