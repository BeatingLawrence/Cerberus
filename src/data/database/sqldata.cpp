#include "sqldata.h"
#include <stdarg.h>
#include <inttypes.h>
#include <cstdlib>
#include "../../core/cerberusutils.h"
#include "../../core/cerberuslog.h"

using namespace cerberus::data::database;

//=============================================================================
SQLRow::~SQLRow()
{
}
//=============================================================================
SQLRow& SQLRow::operator=(const SQLRow& other)
{
    m_values = other.m_values;
    return *this;
}
//=============================================================================
void SQLRow::append(const ::std::string& value)
{
    m_values.push_back(value);
}
//=============================================================================
size_t SQLRow::size() const
{
    return m_values.size();
}
//=============================================================================
void SQLRow::clear()
{
    m_values.clear();
}
//=============================================================================
SQLCell SQLRow::operator [](size_t pos) const
{
    return m_values[pos];
}
//=============================================================================
SQLTablePrototype::SQLTablePrototype(const std::string& name) : m_name(name)
{
    // noop
}
//=============================================================================
SQLTablePrototype& SQLTablePrototype::add(const std::string& name, SQLDataType type, int mod)
{
    m_types.push_back(std::tuple<std::string, SQLDataType, int>(name, type, mod));
    return *this;
}
//=============================================================================
void SQLTablePrototype::clear()
{
    m_types.clear();
}
//=============================================================================
SQLTablePrototype::SQLDataType SQLTablePrototype::toSQLDataType(const std::string& type)
{
    if(type.compare("bigint") == 0)
    {
        return SQLDataType::SDT_BigInt;
    }
    else if(type.compare("integer") == 0)
    {
        return SQLDataType::SDT_Int;
    }
    else if(type.compare("smallint") == 0)
    {
        return SQLDataType::SDT_SmallInt;
    }
    else if(type.compare("real") == 0)
    {
        return SQLDataType::SDT_Real;
    }
    else if(type.compare("double precision") == 0)
    {
        return SQLDataType::SDT_Double;
    }
    else if(type.compare("boolean") == 0)
    {
        return SQLDataType::SDT_Boolean;
    }
    else if(type.compare("money") == 0)
    {
        return SQLDataType::SDT_Money;
    }
    else if(core::CerberusUtils::contains(type, "character"))
    {
        if(core::CerberusUtils::contains(type, "varying"))
        {
            return SQLDataType::SDT_VarChar;
        }
        else
        {
            return SQLDataType::SDT_Char;
        }
    }
    else if(core::CerberusUtils::contains(type, "bit"))
    {
        if(core::CerberusUtils::contains(type, "varying"))
        {
            return SQLDataType::SDT_VarBit;
        }
        else
        {
            return SQLDataType::SDT_Bit;
        }
    }

    return SQLDataType::SDT_Undefined;
}
//=============================================================================
std::string SQLTablePrototype::fromSQLDataType(SQLDataType type)
{
    switch(type)
    {
        case SQLDataType::SDT_Undefined:
            return "";
            break;

        case SQLDataType::SDT_Int:
            return "integer";
            break;

        case SQLDataType::SDT_SmallInt:
            return "smallint";
            break;

        case SQLDataType::SDT_BigInt:
            return "bigint";
            break;

        case SQLDataType::SDT_Real:
            return "real";
            break;

        case SQLDataType::SDT_Double:
            return "double precision";
            break;

        case SQLDataType::SDT_Boolean:
            return "boolean";
            break;

        case SQLDataType::SDT_Bit:
            return "bit";
            break;

        case SQLDataType::SDT_VarBit:
            return "bit varying";
            break;

        case SQLDataType::SDT_Char:
            return "char";
            break;

        case SQLDataType::SDT_VarChar:
            return "char varying";
            break;

        case SQLDataType::SDT_Money:
            return "money";
            break;
    }

    return "";
}
//=============================================================================
SQLBlock::SQLBlock() : m_prototype("") {}
//=============================================================================
SQLBlock::SQLBlock(const std::string& name) : m_prototype(name) {}
//=============================================================================
SQLBlock::~SQLBlock() {}
//=============================================================================
bool SQLBlock::isFailed() const
{
    return m_failed;
}
//=============================================================================
std::string SQLBlock::failureReason() const
{
    return m_failureReason;
}
//=============================================================================
bool SQLBlock::append(const SQLRow& row)
{
    if(structured())
        if(row.size() != m_prototype.m_types.size())
        {
            logError("Refusing to append a row to a block of different structure");
            return false;
        }

    m_rows.push_back(row);
    return true;
}
//=============================================================================
size_t SQLBlock::size() const
{
    return m_rows.size();
}
//=============================================================================
bool SQLBlock::empty() const
{
    return size() == 0;
}
//=============================================================================
void SQLBlock::clear()
{
    clearRows();
    clearStructure();
    m_failureReason = "";
    m_failed = false;
}
//=============================================================================
bool SQLBlock::structured()
{
    return (m_prototype.m_types.size() != 0);
}
//=============================================================================
void SQLBlock::clearStructure()
{
    m_prototype.m_types.clear();
}
//=============================================================================
void SQLBlock::clearRows()
{
    m_rows.clear();
}
//=============================================================================
void SQLBlock::setPrototype(const SQLTablePrototype& prototype)
{
    m_prototype = prototype;
}
//=============================================================================
SQLBlock& SQLBlock::addColumn(const ::std::string& name, SQLTablePrototype::SQLDataType type, int mod)
{
    m_prototype.add(name, type, mod);
    return *this;
}
//=============================================================================
SQLRow SQLBlock::operator[](size_t pos) const
{
    return m_rows[pos];
}
//=============================================================================
SQLBlock::BlockIterator SQLBlock::begin()
{
    return &(*m_rows.begin());
}
//=============================================================================
SQLBlock::BlockIterator SQLBlock::end()
{
    return &(*m_rows.end());
}
//=============================================================================
bool SQLBlock::operator==(const SQLBlock& other) const
{
    if(size() != other.size() || m_prototype.m_types.size() != other.m_prototype.m_types.size())  //be sure rows and columns numbers are equal
    {
        return false;
    }

    for(size_t i = 0; i < size(); i++)
    {
        for(size_t j = 0; j < m_prototype.m_types.size(); j++)
        {
            if(m_rows[i][j].isEqual(other.m_rows[i][j]) != 0)
            {
                return false;
            }
        }
    }

    return true;
}
//=============================================================================
bool SQLBlock::operator!=(const SQLBlock& other) const
{
    return !((*this) == other);
}
//=============================================================================
SQLCell::SQLCell(const std::string& raw) : m_value(raw)
{
}
//=============================================================================
void SQLCell::set(const std::string& value)
{
    m_value = value;
}
//=============================================================================
void SQLCell::set(int64_t value)
{
    m_value = cerberus::core::CerberusUtils::strPrint("%" PRId64, value);   //try "%lld" if it does not work !!!
}
//=============================================================================
void SQLCell::set(double value)
{
    m_value = cerberus::core::CerberusUtils::strPrint("%lf", value);
}
//=============================================================================
void SQLCell::set(bool value)
{
    if(value)
    {
        m_value = "true";
    }
    else
    {
        m_value = "false";
    }
}
//=============================================================================
void SQLCell::set(const std::vector<bool>& value)
{
    m_value = "";

    for(auto&& el : value)
    {
        if(el)
        {
            m_value += "1";
        }
        else
        {
            m_value += "0";
        }
    }
}
//=============================================================================
std::string SQLCell::raw()
{
    return m_value;
}
//=============================================================================
int64_t SQLCell::toInt()
{
    int64_t ret = strtoll(m_value.c_str(), nullptr, 10);

    if(errno == ERANGE)
    {
        logError("INT64 limit reached during string to int conversion");
        return 0;
    }

    return ret;
}
//=============================================================================
double SQLCell::toFloat()
{
    double ret = strtod(m_value.c_str(), nullptr);

    if(errno == ERANGE)
    {
        logError("double limit reached during string to double conversion");
        return 0.0f;
    }

    return ret;
}
//=============================================================================
bool SQLCell::toBool()
{
    auto str = cerberus::core::CerberusUtils::toLower(m_value);

    if(cerberus::core::CerberusUtils::contains(str, "true"))
    {
        return true;
    }
    else if(cerberus::core::CerberusUtils::contains(str, "false"))
    {
        return false;
    }

    logError("called toBool() on a non-boolean SQLCell");
    return false;
}
//=============================================================================
std::vector<bool> SQLCell::toBits()
{
    std::vector<bool> ret;

    for(auto&& el : m_value)
    {
        if(el == '0')
        {
            ret.push_back(false);
        }
        else if(el == '1')
        {
            ret.push_back(true);
        }
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
bool SQLCell::isEqual(const SQLCell& other)
{
    return m_value.compare(other.m_value) == 0;
}
//=============================================================================
bool SQLCell::isEqual(const std::string& str)
{
    return m_value.compare(str) == 0;
}
//=============================================================================
