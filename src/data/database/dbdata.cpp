#include "dbdata.h"

#include <cstdlib>
#include <limits>
#include <sstream>

#include "../../exception/exception.h"

using namespace crb;

namespace
{
    bool dbTypeUsesMod(DBDataType type)
    {
        return type == DDT_Bit || type == DDT_VarBit || type == DDT_Char || type == DDT_VarChar;
    }

    bool validateCellForColumn(const DBCell& cell, const DBColumn& col)
    {
        const SIZE sz      = cell.raw().size();
        const DBDataType t = col.type();
        const DBMOD mod    = col.mod();

        switch (t)
        {
            case DDT_BigInt:
                return sz >= 8;
            case DDT_Double:
                return sz >= 8;

            case DDT_Int:
                return sz >= 4;
            case DDT_Real:
                return sz >= 4;
            case DDT_Money:
                return sz >= 4;

            case DDT_SmallInt:
                return sz >= 2;

            case DDT_Boolean:
                return sz >= 1;

            case DDT_Bit:
                return (mod == 0) || (sz <= CerberusUtils::qceil(mod, 8));
            case DDT_VarBit:
                return (mod == 0) || (sz <= CerberusUtils::qceil(mod, 8));
            case DDT_Char:
                return (mod == 0) || (sz <= mod);
            case DDT_VarChar:
                return (mod == 0) || (sz <= mod);

            default:
                break;
        }

        return false;
    }

    DBCell defaultCellForType(DBDataType type, DBMOD mod)
    {
        switch (type)
        {
            case DDT_BigInt:
            case DDT_Int:
            case DDT_SmallInt:
            case DDT_Money:
                return DBCell((int64_t)0);
            case DDT_Real:
            case DDT_Double:
                return DBCell((long double)0.0);
            case DDT_Boolean:
                return DBCell(false);
            case DDT_Bit:
            case DDT_VarBit:
                return DBCell(std::vector<bool>(mod, false));
            case DDT_Char:
            {
                ByteBuffer raw(mod, 0);
                return DBCell(raw);
            }
            case DDT_VarChar:
                return DBCell(std::string(""));
            default:
                return DBCell();
        }
    }

    std::string trimCopy(const std::string& str)
    {
        return CerberusUtils::removeBlank_copy(str);
    }
}  // namespace

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
    DBMOD cleanMod = mod;

    switch (type)
    {
        case DDT_BigInt:
            return m_value.trim(8);

        case DDT_Double:
            return m_value.trim(8);

        case DDT_Int:
        {
            int64_t v = toInt();
            if (v < std::numeric_limits<int32_t>::min() || v > std::numeric_limits<int32_t>::max())
                throw cIllegalArgExc("integer out of range for DDT_Int");
            return m_value.trim(4);
        }
        case DDT_Real:
        {
            return m_value.trim(4);
        }
        case DDT_Money:
        {
            int64_t v = toInt();
            if (v < std::numeric_limits<int32_t>::min() || v > std::numeric_limits<int32_t>::max())
                throw cIllegalArgExc("integer out of range for DDT_Money");
            return m_value.trim(4);
        }

        case DDT_SmallInt:
        {
            int64_t v = toInt();
            if (v < std::numeric_limits<int16_t>::min() || v > std::numeric_limits<int16_t>::max())
                throw cIllegalArgExc("integer out of range for DDT_SmallInt");
            return m_value.trim(2);
        }

        case DDT_Boolean:
            return m_value.trim(1);

        case DDT_Bit:
        {
            ByteBuffer out = m_value;
            LSIZE req      = CerberusUtils::qceil(cleanMod, 8);
            if (out.size() > req) out = out.trim(req);
            if (out.size() < req) out.append(ByteBuffer(req - out.size(), 0));
            return out;
        }

        case DDT_Char:
        {
            ByteBuffer out = m_value;
            if (cleanMod == 0) return out;
            if (out.size() > cleanMod) out = out.trim(cleanMod);
            if (out.size() < cleanMod) out.append(ByteBuffer(cleanMod - out.size(), 0));
            return out;
        }

        case DDT_VarBit:
        {
            SIZE s = m_value.size();
            ByteBuffer bb;
            bb.appendFrom(&s, CerberusUtils::reqBytes(s));
            return bb.append(m_value.trim(CerberusUtils::qceil(s, 8)));
        }

        case DDT_VarChar:
        {
            // logDebug("varchar ser [%s] %s", m_value.toString().c_str(), m_value.toHex().c_str());
            uint8_t req = CerberusUtils::reqBytes(cleanMod);
            ByteBuffer bb(req, 0);
            SIZE s = m_value.size() > cleanMod ? cleanMod : m_value.size();
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
    switch (m_value.size())
    {
        case 8:
        {
            int64_t v = 0;
            m_value.copyTo(&v, sizeof(v));
            ret = v;
            break;
        }
        case 4:
        {
            int32_t v = 0;
            m_value.copyTo(&v, sizeof(v));
            ret = static_cast<int64_t>(v);
            break;
        }
        case 2:
        {
            int16_t v = 0;
            m_value.copyTo(&v, sizeof(v));
            ret = static_cast<int64_t>(v);
            break;
        }
        case 1:
        {
            int8_t v = 0;
            m_value.copyTo(&v, sizeof(v));
            ret = static_cast<int64_t>(v);
            break;
        }
        default:
        {
            m_value.copyTo(&ret, sizeof(ret));
            break;
        }
    }
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
DBQuery::DBQuery(const std::string& tableName)
    : m_table(tableName),
      m_condition(),
      m_columns()
{
}
//=============================================================================
const std::string& DBQuery::table() const { return m_table; }
//=============================================================================
void DBQuery::setTable(const std::string& tableName) { m_table = tableName; }
//=============================================================================
const DBQueryCondition& DBQuery::condition() const { return m_condition; }
//=============================================================================
DBQueryCondition& DBQuery::condition() { return m_condition; }
//=============================================================================
const std::vector<std::string>& DBQuery::columns() const { return m_columns; }
//=============================================================================
void DBQuery::setColumns(const std::vector<std::string>& columns) { m_columns = columns; }
//=============================================================================
void DBQuery::addColumn(const std::string& column) { m_columns.push_back(column); }
//=============================================================================
bool DBQuery::selectAllColumns() const { return m_columns.empty(); }
//=============================================================================
OpRes DBQuery::validate() const
{
    if (trimCopy(m_table).empty()) return {OR_WrongArgument, "query table name is empty"};
    if (m_condition.type == DBQC_None) return {OR_WrongArgument, "query condition is missing"};
    if (trimCopy(m_condition.column).empty()) return {OR_WrongArgument, "query condition column is empty"};

    if (m_condition.type == DBQC_Regex && m_condition.regexPattern.empty())
        return {OR_WrongArgument, "query regex pattern is empty"};

    if (m_condition.type == DBQC_Exact && m_condition.exactValue.empty())
        return {OR_WrongArgument, "query exact pattern is empty"};

    if (m_condition.type == DBQC_Range && !m_condition.min.has_value() && !m_condition.max.has_value())
        return {OR_WrongArgument, "query range has no limits"};

    return OR_OK;
}
//=============================================================================
OpResData<DBQuery> DBQuery::fromString(const std::string& query)
{
    std::string q = trimCopy(query);
    if (q.empty()) return {OR_WrongArgument, "query is empty"};

    auto firstColon = q.find(':');
    if (firstColon == std::string::npos) return {OR_WrongArgument, "bad query format: missing table separator"};

    DBQuery out(trimCopy(q.substr(0, firstColon)));
    if (out.table().empty()) return {OR_WrongArgument, "bad query format: empty table name"};

    std::string tail = q.substr(firstColon + 1);
    if (tail.empty()) return {OR_WrongArgument, "bad query format: missing row selection pattern"};

    std::string patternPart;
    std::string columnsPart;

    auto columnsPos = tail.rfind(":[");
    if (columnsPos != std::string::npos && tail.back() == ']')
    {
        patternPart = tail.substr(0, columnsPos);
        columnsPart = tail.substr(columnsPos + 1);
    }
    else
        patternPart = tail;

    patternPart = trimCopy(patternPart);
    if (patternPart.empty()) return {OR_WrongArgument, "bad query format: empty row selection pattern"};

    DBQueryCondition cond;
    if (patternPart.front() == '[')
    {
        auto rb = patternPart.find(']');
        if (rb == std::string::npos) return {OR_WrongArgument, "bad range/exact pattern"};

        std::string body = patternPart.substr(1, rb - 1);
        cond.column      = trimCopy(patternPart.substr(rb + 1));
        if (cond.column.empty()) return {OR_WrongArgument, "missing column in range/exact pattern"};

        auto sep = body.find(':');
        if (sep == std::string::npos)
        {
            cond.type       = DBQC_Exact;
            cond.exactValue = trimCopy(body);
            if (cond.exactValue.empty()) return {OR_WrongArgument, "empty exact pattern"};
        }
        else
        {
            cond.type = DBQC_Range;
            std::string mn = trimCopy(body.substr(0, sep));
            std::string mx = trimCopy(body.substr(sep + 1));
            if (!mn.empty())
            {
                try
                {
                    cond.min = std::stold(mn);
                }
                catch (...)
                {
                    return {OR_WrongArgument, "invalid minimum range value"};
                }
            }
            if (!mx.empty())
            {
                try
                {
                    cond.max = std::stold(mx);
                }
                catch (...)
                {
                    return {OR_WrongArgument, "invalid maximum range value"};
                }
            }
            if (!cond.min.has_value() && !cond.max.has_value())
                return {OR_WrongArgument, "range pattern requires min, max, or both"};

            if (cond.min.has_value() && cond.max.has_value() && cond.min.value() > cond.max.value())
                cond.invertOrder = true;
        }
    }
    else if (patternPart.front() == '{')
    {
        auto rb = patternPart.find('}');
        if (rb == std::string::npos) return {OR_WrongArgument, "bad regex pattern"};
        cond.type         = DBQC_Regex;
        cond.regexPattern = patternPart.substr(1, rb - 1);
        cond.column       = trimCopy(patternPart.substr(rb + 1));
        if (cond.regexPattern.empty()) return {OR_WrongArgument, "empty regex pattern"};
        if (cond.column.empty()) return {OR_WrongArgument, "missing column in regex pattern"};
    }
    else
        return {OR_WrongArgument, "unknown row selection pattern"};

    out.condition() = cond;

    if (!columnsPart.empty())
    {
        if (columnsPart.front() != '[' || columnsPart.back() != ']')
            return {OR_WrongArgument, "bad columns format"};

        std::string cols = columnsPart.substr(1, columnsPart.size() - 2);
        if (trimCopy(cols).empty()) return {OR_WrongArgument, "empty columns selection"};

        size_t pos = 0;
        while (pos < cols.size())
        {
            auto comma = cols.find(',', pos);
            std::string col =
                cols.substr(pos, comma == std::string::npos ? std::string::npos : comma - pos);
            col = trimCopy(col);
            if (col.empty()) return {OR_WrongArgument, "empty column in columns selection"};
            out.addColumn(col);
            if (comma == std::string::npos) break;
            pos = comma + 1;
        }
    }

    auto ok = out.validate();
    if (ok.fail()) return ok;
    return out;
}
//=============================================================================
//=============================================================================
//=============================================================================
DBRow::DBRow()
    : m_values(),
      m_protoRef(nullptr),
      m_protoOwned()
{
}
//=============================================================================
DBRow::DBRow(const DBRow& other)
    : m_values(other.m_values),
      m_protoRef(nullptr),
      m_protoOwned()
{
    if (other.m_protoRef != nullptr)
    {
        m_protoOwned = std::make_unique<DBTableProto>(*other.m_protoRef);
        m_protoRef   = m_protoOwned.get();
    }
}
//=============================================================================
DBRow& DBRow::operator=(const DBRow& other)
{
    if (this == &other) return *this;

    m_values = other.m_values;
    m_protoOwned.reset();
    m_protoRef = nullptr;

    if (other.m_protoRef != nullptr)
    {
        m_protoOwned = std::make_unique<DBTableProto>(*other.m_protoRef);
        m_protoRef   = m_protoOwned.get();
    }

    return *this;
}
//=============================================================================
void DBRow::bindPrototype(const DBTableProto* proto)
{
    m_protoOwned.reset();
    m_protoRef = proto;
}
//=============================================================================
void DBRow::detachPrototype()
{
    m_protoOwned.reset();
    m_protoRef = nullptr;
}
//=============================================================================
bool DBRow::hasPrototype() const { return m_protoRef != nullptr; }
//=============================================================================
const DBTableProto* DBRow::prototype() const { return m_protoRef; }
//=============================================================================
DBRow::~DBRow() {}
//=============================================================================
void DBRow::append(const DBCell& value)
{
    if (m_protoRef != nullptr)
    {
        if (m_values.size() >= m_protoRef->size())
            throw cIllegalArgExc("row append overflow: too many fields for bound prototype");

        const DBColumn& col = (*m_protoRef)[m_values.size()];
        if (!validateCellForColumn(value, col))
            throw cIllegalArgExc("row append type mismatch for column %s", col.name().c_str());
    }

    m_values.push_back(value);
}
//=============================================================================
size_t DBRow::size() const { return m_values.size(); }
//=============================================================================
void DBRow::clear() { m_values.clear(); }
//=============================================================================
bool DBRow::verify(const DBTableProto& proto) const
{
    if (m_values.size() > proto.size()) return false;

    for (size_t i = 0; i < m_values.size(); ++i)
    {
        if (!validateCellForColumn(m_values[i], proto[i])) return false;
    }

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

        // logDebug("serializing cell %u %u %u %s", i, proto[i].type(), bb.size(), bb.toHex().c_str());

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
const DBCell& DBRow::operator[](const std::string& column) const
{
    if (m_protoRef == nullptr) throw cIllegalStateExc("row has no bound prototype");
    int idx = m_protoRef->columnIndex(column);
    if (idx < 0) throw cIllegalArgExc("column not found: %s", column.c_str());
    return m_values.at(static_cast<size_t>(idx));
}
//=============================================================================
DBCell& DBRow::operator[](const std::string& column)
{
    if (m_protoRef == nullptr) throw cIllegalStateExc("row has no bound prototype");
    int idx = m_protoRef->columnIndex(column);
    if (idx < 0) throw cIllegalArgExc("column not found: %s", column.c_str());
    return m_values.at(static_cast<size_t>(idx));
}
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
DBTableProto& DBTableProto::add(const std::string& name, DBDataType type, DBMOD mod, DBFLAGS flags)
{
    DBMOD cleanMod = dbTypeUsesMod(type) ? mod : 0;
    DBCell def     = defaultCellForType(type, cleanMod);
    m_types.push_back({name, type, cleanMod, flags, def});
    return *this;
}
//=============================================================================
DBTableProto& DBTableProto::add(const std::string& name, DBDataType type, DBMOD mod, DBFLAGS flags,
                                const DBCell& defaultValue)
{
    DBMOD cleanMod = dbTypeUsesMod(type) ? mod : 0;
    m_types.push_back({name, type, cleanMod, flags, defaultValue});
    return *this;
}
//=============================================================================
OpRes DBTableProto::setPrimaryKey(const std::string& name)
{
    for (size_t i = 0; i < m_types.size(); ++i)
        if (m_types[i].name() == name) return setPrimaryKey(i);
    return OR_NotFound;
}
//=============================================================================
OpRes DBTableProto::setPrimaryKey(size_t index)
{
    if (index >= m_types.size()) return OR_WrongArgument;

    for (auto& c : m_types)
        c = DBColumn(c.name(), c.type(), c.mod(), c.flags() & ~DBF_PrimaryKey, c.defaultValue());

    auto& col = m_types[index];
    col       = DBColumn(col.name(), col.type(), col.mod(), col.flags() | DBF_PrimaryKey, col.defaultValue());
    return OR_OK;
}
//=============================================================================
int DBTableProto::primaryKeyIndex() const
{
    for (size_t i = 0; i < m_types.size(); ++i)
        if (m_types[i].isPrimary()) return static_cast<int>(i);
    return -1;
}
//=============================================================================
int DBTableProto::columnIndex(const std::string& name) const
{
    for (size_t i = 0; i < m_types.size(); ++i)
        if (m_types[i].name() == name) return static_cast<int>(i);
    return -1;
}
//=============================================================================
OpRes DBTableProto::renameColumn(const std::string& oldName, const std::string& newName)
{
    std::string oldN = trimCopy(oldName);
    std::string newN = trimCopy(newName);
    if (oldN.empty() || newN.empty()) return {OR_WrongArgument, "column name cannot be empty"};
    if (oldN == newN) return OR_OK;

    int from = columnIndex(oldN);
    if (from < 0) return {OR_NotFound, "column not found: " + oldN};
    if (columnIndex(newN) >= 0) return {OR_AlreadyPresent, "column already present: " + newN};

    m_types[from].setName(newN);
    return OR_OK;
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
    if (!structured())
    {
        DBRow nr = row;
        nr.detachPrototype();
        m_rows.push_back(nr);
        return OR_OK;
    }

    if (row.size() > m_prototype.size())
        return {OR_WrongArgument, "row has more columns than prototype", std::to_string(row.size())};

    DBRow normalized;
    normalized.bindPrototype(&m_prototype);

    try
    {
        for (size_t i = 0; i < row.size(); ++i) normalized.append(row[i]);
        for (size_t i = row.size(); i < m_prototype.size(); ++i) normalized.append(m_prototype[i].defaultValue());
    }
    catch (const std::exception& e)
    {
        return {OR_WrongType, "row append validation failed", e.what()};
    }

    m_rows.push_back(normalized);
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
void DBTableBlock::setPrototype(const DBTableProto& prototype)
{
    m_prototype = prototype;
    for (auto& row : m_rows)
    {
        row.bindPrototype(&m_prototype);
        for (size_t i = row.size(); i < m_prototype.size(); ++i) row.append(m_prototype[i].defaultValue());
    }
}
//=============================================================================
void DBTableBlock::assignRows(const DBTableBlock& other)
{
    m_rows = other.m_rows;
    if (structured())
        for (auto& row : m_rows) row.bindPrototype(&m_prototype);
    else
        for (auto& row : m_rows) row.detachPrototype();
}
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
DBTableBlock& DBTableBlock::addColumn(const ::std::string& name, DBDataType type, DBMOD mod, DBFLAGS flags)
{
    m_prototype.add(name, type, mod, flags);
    return *this;
}
//=============================================================================
DBTableBlock& DBTableBlock::addColumn(const std::string& name, DBDataType type, DBMOD mod, DBFLAGS flags,
                                      const DBCell& defaultValue)
{
    m_prototype.add(name, type, mod, flags, defaultValue);
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
std::string DBTableBlock::toString() const
{
    std::stringstream ss;
    if (m_prototype.size() > 0)
    {
        for (size_t i = 0; i < m_prototype.size(); ++i)
        {
            const auto& col = m_prototype[i];
            std::string t   = CerberusUtils::fromDBDataType(col.type());
            std::string modStr;
            switch (col.type())
            {
                case DDT_Bit:
                case DDT_VarBit:
                case DDT_Char:
                case DDT_VarChar:
                    modStr = std::to_string(col.mod());
                    break;
                default:
                    break;
            }
            std::string flagStr;
            if (col.isPrimary()) flagStr.push_back('P');

            ss << col.name() << " ";
            bool hasParen = (!modStr.empty() || !flagStr.empty());
            if (hasParen)
                ss << t << "(";
            else
                ss << t;

            if (!modStr.empty()) ss << modStr;
            if (!flagStr.empty()) ss << flagStr;
            if (hasParen) ss << ")";

            if (i + 1 < m_prototype.size()) ss << " | ";
        }
        ss << "\n";
    }

    auto cellStr = [&](const DBCell& c, DBDataType t) -> std::string
    {
        switch (t)
        {
            case DDT_BigInt:
            case DDT_Int:
            case DDT_SmallInt:
            case DDT_Money:
                return std::to_string(c.toInt());
            case DDT_Double:
            case DDT_Real:
                return std::to_string((double)c.toReal());
            case DDT_Boolean:
                return c.toBool() ? "true" : "false";
            case DDT_Char:
            case DDT_VarChar:
                return "'" + c.raw().toString() + "'";
            case DDT_Bit:
            case DDT_VarBit:
                return c.raw().toHex();
            default:
                return c.raw().toString();
        }
    };

    for (auto&& row : m_rows)
    {
        for (size_t i = 0; i < row.size(); ++i)
        {
            DBDataType t = m_prototype.size() > i ? m_prototype[i].type() : DDT_Int;
            ss << cellStr(row[i], t);
            if (i + 1 < row.size()) ss << " | ";
        }
        ss << "\n";
    }

    return ss.str();
}
//=============================================================================
bool DBColumn::operator==(const DBColumn& other) const
{
    return m_columnName == other.m_columnName && m_type == other.m_type && m_mod == other.m_mod &&
           m_flags == other.m_flags && m_defaultValue.isEqual(other.m_defaultValue);
}
//=============================================================================
