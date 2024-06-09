#include "dbdata.h"

#include <inttypes.h>

#include <cstdlib>

#include "../../core/cerberusutils.h"
#include "src/cerberus.h"

using namespace cerberus;

//=============================================================================
DBRow::~DBRow() {}
//=============================================================================
void DBRow::append(const DBCell& value) { m_values.push_back(value); }
//=============================================================================
size_t DBRow::size() const { return m_values.size(); }
//=============================================================================
void DBRow::clear() { m_values.clear(); }
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
DBTableProto::DBTableProto(const std::string& name)
    : m_name(name)
{
    // noop
}
//=============================================================================
DBTableProto& DBTableProto::add(const std::string& name, DBDataType type, int mod)
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
std::string DBTableProto::name() const { return m_name; }
//=============================================================================
Iterator<DBColumn> DBTableProto::begin() { return &(*m_types.begin()); }
//=============================================================================
Iterator<DBColumn> DBTableProto::end() { return &(*m_types.end()); }
//=============================================================================
ConstIterator<DBColumn> DBTableProto::begin() const { return &(*m_types.begin()); }
//=============================================================================
ConstIterator<DBColumn> DBTableProto::end() const { return &(*m_types.end()); }
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
const DBTableProto& DBTableBlock::prototype() const { return m_prototype; }
//=============================================================================
DBTableBlock& DBTableBlock::addColumn(const ::std::string& name, DBDataType type, int mod)
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
DBCell::DBCell(const std::string& raw)
    : m_value(raw)
{
}
//=============================================================================
DBCell::DBCell(const char* value)
    : m_value(value)
{
}
//=============================================================================
DBCell::DBCell(int64_t value) { set(value); }
//=============================================================================
DBCell::DBCell(int value) { set(int64_t(value)); }
//=============================================================================
DBCell::DBCell(unsigned int value) { set(int64_t(value)); }
//=============================================================================
DBCell::DBCell(double value) { set(value); }
//=============================================================================
DBCell::DBCell(float value) { set((double)value); }
//=============================================================================
DBCell::DBCell(bool value) { set(value); }
//=============================================================================
DBCell::DBCell(const std::vector<bool>& value) { set(value); }
//=============================================================================
void DBCell::set(const std::string& value) { m_value = value; }
//=============================================================================
void DBCell::set(int64_t value) { m_value = CerberusUtils::strPrint("%" PRId64, value); }
//=============================================================================
void DBCell::set(double value) { m_value = CerberusUtils::strPrint("%lf", value); }
//=============================================================================
void DBCell::set(bool value)
{
    if (value)
        m_value = "true";
    else
        m_value = "false";
}
//=============================================================================
void DBCell::set(const std::vector<bool>& value)
{
    m_value = "";

    for (auto&& el : value)
    {
        if (el)
            m_value += "1";
        else
            m_value += "0";
    }
}
//=============================================================================
std::string DBCell::raw() const { return m_value; }
//=============================================================================
int64_t DBCell::toInt() const
{
    int64_t ret = strtoll(m_value.c_str(), nullptr, 10);

    if (errno == ERANGE)
    {
        logError("INT64 limit reached during string to int conversion");
        return 0;
    }

    return ret;
}
//=============================================================================
double DBCell::toFloat() const
{
    double ret = strtod(m_value.c_str(), nullptr);

    if (errno == ERANGE)
    {
        logError("double limit reached during string to double conversion");
        return 0.0f;
    }

    return ret;
}
//=============================================================================
bool DBCell::toBool() const
{
    auto str = CerberusUtils::toLower(m_value);

    if (str.compare("t") == 0 || str.compare("true") == 0)
        return true;

    else if (str.compare("f") == 0 || str.compare("false") == 0)
        return false;

    logError("called toBool() on a non-boolean SQLCell, value %s not recognized", m_value.c_str());
    return false;
}
//=============================================================================
std::vector<bool> DBCell::toBits() const
{
    std::vector<bool> ret;

    for (auto&& el : m_value)
    {
        if (el == '0')
            ret.push_back(false);

        else if (el == '1')
            ret.push_back(true);

        else
        {
            logError("called toBits() on a SQLCell that does not contain any bit");
            std::vector<bool> fail;
            return fail;
        }
    }

    return ret;
}
//=============================================================================
bool DBCell::isEqual(const DBCell& other) const { return m_value.compare(other.m_value) == 0; }
//=============================================================================
bool DBCell::isEqual(const std::string& str) const { return m_value.compare(str) == 0; }
//=============================================================================
