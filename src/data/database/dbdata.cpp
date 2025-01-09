#include "dbdata.h"

#include <cstdlib>

#include "../../cerberus.h"
#include "../../exception/exception.h"

using namespace cerberus;

//=============================================================================
void DBCell::_fromBitArray(const std::vector<bool>& arr)
{
    m_value.clear();

    bitmask mask;
    uint8_t i = 0;

    for (auto&& el : arr)
    {
        mask.set(i, el);

        if (i == 7)
        {
            i = 0;
            m_value.appendChar(mask.bits);
            mask.reset();
        }
        else
            i++;
    }

    if (i) m_value.appendChar(mask.bits);
}
//=============================================================================
DBCell::DBCell(int64_t value)
    : m_value((BYTE*)&value, sizeof(value))
{
}
//=============================================================================
DBCell::DBCell(int32_t value)
    : m_value((BYTE*)&value, sizeof(value))
{
}
//=============================================================================
DBCell::DBCell(float value)
    : m_value((BYTE*)&value, sizeof(value))
{
}
//=============================================================================
DBCell::DBCell(long double value)
    : m_value((BYTE*)&value, sizeof(value))
{
}
//=============================================================================
DBCell::DBCell(bool value)
    : m_value((BYTE*)&value, sizeof(value))
{
}
//=============================================================================
DBCell::DBCell(const std::vector<bool>& value)
    : m_value()
{
    _fromBitArray(value);
}
//=============================================================================
DBCell::DBCell(const std::string& str)
    : m_value(str)
{
}
//=============================================================================
DBCell::DBCell(const char* str)
    : m_value(str)
{
}
//=============================================================================
DBCell::DBCell(const ByteBuffer& raw)
    : m_value(raw)
{
}
//=============================================================================
void DBCell::set(const std::string& str) { m_value = str; }
//=============================================================================
void DBCell::set(const char* str) { m_value = str; }
//=============================================================================
void DBCell::set(int64_t value) { m_value.assignFrom((BYTE*)&value, sizeof(value)); }
//=============================================================================
void DBCell::set(long double value) { m_value.assignFrom((BYTE*)&value, sizeof(value)); }
//=============================================================================
void DBCell::set(bool value) { m_value.assignFrom((BYTE*)&value, sizeof(value)); }
//=============================================================================
void DBCell::set(const std::vector<bool>& value) { _fromBitArray(value); }
//=============================================================================
ByteBuffer& DBCell::raw() { return m_value; }
//=============================================================================
const ByteBuffer& DBCell::raw() const { return m_value; }
//=============================================================================
ByteBuffer DBCell::serialize(DBDataType type, DBMOD mod) const
{
    switch (type)
    {
        case DDT_BigInt:
        case DDT_Double:
            return m_value.trim(8);

        case DDT_Int:
        case DDT_Real:
        case DDT_Money:
            return m_value.trim(4);

        case DDT_SmallInt:
            return m_value.trim(2);

        case DDT_Boolean:
            return m_value.trim(1);

        case DDT_Bit:
        case DDT_Char:
            return m_value;

        case DDT_VarBit:
        {
            SIZE s = m_value.size();
            ByteBuffer bb;
            bb.appendFrom(&s, CerberusUtils::reqBytes(s));
            return bb.append(m_value.trim(CerberusUtils::qceil(s, 8)));
        }

        case DDT_VarChar:
        {
            logDebug("varchar ser [%s] %s", m_value.toString().c_str(), m_value.toHex().c_str());
            uint8_t req = CerberusUtils::reqBytes(mod);
            ByteBuffer bb(req, 0);
            SIZE s = m_value.size() > mod ? mod : m_value.size();
            bb.copyFrom(&s, req);
            bb.append(m_value);
            return bb;
        }

        default:
            break;
    }

    throw cIllegalArgExc("unknown type");
}
//=============================================================================
int64_t DBCell::toInt() const
{
    int64_t ret = 0;
    m_value.copyTo(&ret, sizeof(ret));
    return ret;
}
//=============================================================================
long double DBCell::toReal() const
{
    long double ret = 0.0f;
    m_value.copyTo(&ret, sizeof(ret));
    return ret;
}
//=============================================================================
bool DBCell::toBool() const
{
    bool ret = false;
    m_value.copyTo(&ret, sizeof(ret));
    return ret;
}
//=============================================================================
std::vector<bool> DBCell::toBits() const
{
    if (m_value.isEmpty()) return std::vector<bool>();

    std::vector<bool> ret;

    for (auto&& el : m_value)
    {
        bitmask mask(el);

        for (int i = 0; i < 8; i++)
        {
            ret.push_back(mask[i]);
        }
    }

    return ret;
}
//=============================================================================
bool DBCell::isEqual(const DBCell& other) const { return m_value == other.m_value; }
//=============================================================================
//=============================================================================
//=============================================================================
DBRow::~DBRow() {}
//=============================================================================
void DBRow::append(const DBCell& value) { m_values.push_back(value); }
//=============================================================================
size_t DBRow::size() const { return m_values.size(); }
//=============================================================================
void DBRow::clear() { m_values.clear(); }
//=============================================================================
bool DBRow::verify(const DBTableProto& proto) const
{
#pragma GCC warning "implement verify method"
    return true;
}
//=============================================================================
ByteBuffer DBRow::serialize(const DBTableProto& proto) const
{
    ByteBuffer buf;
    int i = 0;

    for (auto&& el : *this)
    {
        auto bb = el.serialize(proto[i].type(), proto[i].mod());

        logDebug("serializing cell %u %u %u %s", i, proto[i].type(), bb.size(), bb.toHex().c_str());

        buf.append(bb);
        i++;
    }
    return buf;
}
//=============================================================================
const DBCell& DBRow::operator[](size_t pos) const { return m_values.at(pos); }
//=============================================================================
DBCell& DBRow::operator[](size_t pos) { return m_values.at(pos); }
//=============================================================================
Iterator<DBCell> DBRow::begin() { return &(*m_values.begin()); }
//=============================================================================
Iterator<DBCell> DBRow::end() { return &(*m_values.end()); }
//=============================================================================
ConstIterator<DBCell> DBRow::begin() const { return &(*m_values.begin()); }
//=============================================================================
ConstIterator<DBCell> DBRow::end() const { return &(*m_values.end()); }
//=============================================================================
//=============================================================================
//=============================================================================
DBTableProto::DBTableProto(const std::string& name)
    : m_name(name)
{
    // noop
}
//=============================================================================
DBTableProto& DBTableProto::add(const std::string& name, DBDataType type, DBMOD mod)
{
    m_types.push_back({name, type, mod});
    return *this;
}
//=============================================================================
const DBColumn& DBTableProto::operator[](int index) const { return m_types.at(index); }
//=============================================================================
DBColumn& DBTableProto::operator[](int index) { return m_types.at(index); }
//=============================================================================
void DBTableProto::clear() { m_types.clear(); }
//=============================================================================
size_t DBTableProto::size() const { return m_types.size(); }
//=============================================================================
bool DBTableProto::isEqual(const DBTableProto& other) const
{
    return m_name == other.m_name && m_types == other.m_types;
}
//=============================================================================
std::string DBTableProto::name() const { return m_name; }
//=============================================================================
void DBTableProto::setName(const std::string& name) { m_name = name; }
//=============================================================================
Iterator<DBColumn> DBTableProto::begin() { return &(*m_types.begin()); }
//=============================================================================
Iterator<DBColumn> DBTableProto::end() { return &(*m_types.end()); }
//=============================================================================
ConstIterator<DBColumn> DBTableProto::begin() const { return &(*m_types.begin()); }
//=============================================================================
ConstIterator<DBColumn> DBTableProto::end() const { return &(*m_types.end()); }
//=============================================================================
//=============================================================================
//=============================================================================
DBTableBlock::DBTableBlock(const std::string& name)
    : m_prototype(name)
{
}
//=============================================================================
DBTableBlock::DBTableBlock(const DBTableProto& proto)
    : m_prototype(proto)
{
}
//=============================================================================
DBTableBlock::DBTableBlock(const DBRow& row) { m_rows.push_back(row); }
//=============================================================================
DBTableBlock::~DBTableBlock() {}
//=============================================================================
OpRes DBTableBlock::append(const DBRow& row)
{
    if (structured())
        if (row.size() != m_prototype.size()) return {OR_WrongArgument, "row and proto size mismatch"};

    m_rows.push_back(row);
    return OR_OK;
}
//=============================================================================
size_t DBTableBlock::size() const { return m_rows.size(); }
//=============================================================================
bool DBTableBlock::empty() const { return m_rows.empty(); }
//=============================================================================
void DBTableBlock::clear()
{
    clearRows();
    clearStructure();
}
//=============================================================================
bool DBTableBlock::structured() const { return m_prototype.size(); }
//=============================================================================
void DBTableBlock::clearStructure() { m_prototype.clear(); }
//=============================================================================
void DBTableBlock::clearRows() { m_rows.clear(); }
//=============================================================================
void DBTableBlock::setPrototype(const DBTableProto& prototype) { m_prototype = prototype; }
//=============================================================================
void DBTableBlock::assignRows(const DBTableBlock& other) { m_rows = other.m_rows; }
//=============================================================================
bool DBTableBlock::verify() const { return verify(m_prototype); }
//=============================================================================
bool DBTableBlock::verify(const DBTableProto& proto) const
{
    for (auto&& el : *this)
        if (!el.verify(proto)) return false;

    return true;
}
//=============================================================================
ByteBuffer DBTableBlock::serialize(const DBTableProto& proto) const
{
    ByteBuffer buf;
    for (auto&& el : *this) buf.append(el.serialize(proto));
    return buf;
}
//=============================================================================
const DBTableProto& DBTableBlock::prototype() const { return m_prototype; }
//=============================================================================
DBTableBlock& DBTableBlock::addColumn(const ::std::string& name, DBDataType type, DBMOD mod)
{
    m_prototype.add(name, type, mod);
    return *this;
}
//=============================================================================
const DBRow& DBTableBlock::operator[](size_t pos) const { return m_rows.at(pos); }
//=============================================================================
DBRow& DBTableBlock::operator[](size_t pos) { return m_rows.at(pos); }
//=============================================================================
bool DBTableBlock::operator==(const DBTableBlock& other) const
{
    if (size() != other.size() ||
        m_prototype.size() != other.m_prototype.size())  // be sure rows and columns numbers are equal
    {
        return false;
    }

    for (size_t i = 0; i < size(); i++)
    {
        for (size_t j = 0; j < m_prototype.size(); j++)
        {
            if (m_rows[i][j].isEqual(other.m_rows[i][j]) != 0) return false;
        }
    }

    return true;
}
//=============================================================================
bool DBTableBlock::operator!=(const DBTableBlock& other) const { return !((*this) == other); }
//=============================================================================
Iterator<DBRow> DBTableBlock::begin() { return &(*m_rows.begin()); }
//=============================================================================
Iterator<DBRow> DBTableBlock::end() { return &(*m_rows.end()); }
//=============================================================================
ConstIterator<DBRow> DBTableBlock::begin() const { return &(*m_rows.begin()); }
//=============================================================================
ConstIterator<DBRow> DBTableBlock::end() const { return &(*m_rows.end()); }
//=============================================================================
bool DBColumn::operator==(const DBColumn& other) const
{
    return m_columnName == other.m_columnName && m_type == other.m_type && m_mod == other.m_mod;
}
//=============================================================================
