#include "filesystemdb.h"

// #include "../../cerberus.h"
#include "../../exception/exception.h"
#include <boost/regex.hpp>
#include <numeric>
#include <optional>
#include <unordered_map>
#include <unordered_set>

using namespace crb::db;
using namespace crb;

namespace
{
    OpRes buildInsertRowFromUpdate(const DBRow* src, const DBTableProto& tableProto, const DBTableProto& blockProto,
                                   const std::vector<int>& blockToTable, DBRow& dst)
    {
        (void)blockProto;

        dst.clear();
        dst.bindPrototype(&tableProto);
        for (size_t t = 0; t < tableProto.size(); ++t) dst.append(tableProto[t].defaultValue());

        for (size_t bi = 0; bi < blockToTable.size(); ++bi)
        {
            int tIdx = blockToTable[bi];
            if (tIdx < 0) continue;
            dst[static_cast<size_t>(tIdx)] = (*src)[bi];
        }

        return OR_OK;
    }
}  // namespace

/*  STRUCTURE:
 *
 *  //xyyyyyyyy[...][0]table_name[0]col1_name[0]col2_name[...][0]size buffer
 *    ^                                                               ^
 *    header starts here                                              buffer starts here
 *
 *  x=typeid
 *  y=bytes mod (LSB ... MSB)
 *
 *  size is expressed in number of rows
 *
 */

#define HASH_FUNC(str) CerberusUtils::hash_fnv1a(str)

//=============================================================================
OpResData<string> FilesystemDB::_readStr()
{
    auto n = m_file.readUntil({(SIZE)1, (BYTE)0});  // find [0] delimiter

    if (n.fail()) return n;
    m_file.seekOffset(1);
    return n.value.toString();
}
//=============================================================================
DBMOD FilesystemDB::_getMod(const ByteBuffer& buf)
{
    if (buf.size() != 4) throw cIllegalArgExc("_getMod() called with wrong args");
    DBMOD ret = 0;
    buf.copyTo(&ret, sizeof(ret));
    return ret;
}
//=============================================================================
DBFLAGS _getFlagsFromBuf(const ByteBuffer& buf)
{
    if (buf.size() != 4) throw cIllegalArgExc("_getFlags() called with wrong args");
    DBFLAGS ret = 0;
    buf.copyTo(&ret, sizeof(ret));
    return ret;
}
//=============================================================================
OpRes FilesystemDB::_parseTableHeader(Table& tab)
{
    condret_str(m_file.seek(tab.header), "seek");
    ByteBuffer buf;

    while (true)
    {
        condret(m_file.readChunk(buf, 1));
        DBDataType type = (DBDataType)buf.at(0);

        if (type == 0) break;  // first [0] delimiter found

        condret_str(m_file.readChunk(buf, 4), "error while parsing typeID");
        DBMOD mod = _getMod(buf);

        condret_str(m_file.readChunk(buf, 4), "error while parsing flags");
        DBFLAGS flags = _getFlagsFromBuf(buf);

        tab.proto.add("", type, mod, flags);
    }

    auto str = _readStr().expect();
    condret_str(str, "error while parsing table name");

    tab.proto.setName(str.value);  // table name
    tab.tableID = HASH_FUNC(str.value);

    for (auto&& el : tab.proto)
    {
        str = _readStr().expect();
        condret_str(str, "error while parsing column name");
        el.setName(str.value);
    }

    for (auto&& el : tab.proto)
    {
        auto def = _parseField(el.type(), el.mod());
        condret_str(def, "error while parsing default value");
        el.setDefaultValue(def.value);
    }

    tab.buffer = m_file.getCursor().expect().value + 8;  // skip size field to point to buffer start

    condret_str(_skipTable(tab, nullptr), "error while skipping table data");

    return OR_OK;
}
//=============================================================================
OpRes FilesystemDB::_load()
{
    if (!m_file.isOpen()) return OR_BadConditions;

    m_tables.clear();
    m_file.resetCursor();

    while (true)
    {
        auto res = m_file.search("//");  // search the table start sequence

        if (res.fail())
        {
            if (res.res == OR_NotFound || res.res == OR_EOF) return OR_OK;  // end of database
            return res;
        }

        Table tab;
        tab.header = res.value + 2;  // skip the table start sequence
        condret(_parseTableHeader(tab));

        m_tables.push_back(tab);
    }
}
//=============================================================================
OpRes FilesystemDB::_buildHeader(const DBTableProto& proto, Table& tab)
{
    ByteBuffer buf;
    buf.appendString("//");

    for (auto&& el : proto)
    {
        buf.appendChar(el.type());
        DBMOD mod = el.mod();
        buf.append_4b(&mod);
        DBFLAGS flags = el.flags();
        buf.append_4b(&flags);
    }

    buf.appendChar(0);
    buf.appendString(proto.name());
    buf.appendChar(0);

    for (auto&& el : proto)
    {
        buf.appendString(el.name());
        buf.appendChar(0);
    }

    for (auto&& el : proto)
    {
        ByteBuffer def = el.defaultValue().serialize(el.type(), el.mod());
        if (el.type() == DDT_Char)
        {
            LSIZE req = el.mod();
            if (def.size() < req) def.append(ByteBuffer(req - def.size(), 0));
            if (def.size() > req) def = def.trim(req);
        }
        else if (el.type() == DDT_Bit)
        {
            LSIZE req = CerberusUtils::qceil(el.mod(), 8);
            if (def.size() < req) def.append(ByteBuffer(req - def.size(), 0));
            if (def.size() > req) def = def.trim(req);
        }
        buf.append(def);
    }

    uint64_t size = 0;
    buf.append_8b(&size);  // reserve 8 bytes for the size field

    LSIZE header = m_file.size().expect().value + 2;
    condret(m_file.writeExpand(buf));
    LSIZE buffer = header + buf.size() - 2;

    tab.header = header;
    tab.buffer = buffer;

    return OR_OK;
}
//=============================================================================
OpResData<FilesystemDB::Buf_Size> FilesystemDB::_parseFieldRaw(DBDataType type, DBMOD mod)
{
    ByteBuffer buf;

    switch (type)
    {
        case DDT_BigInt:
        case DDT_Double:
        {
            condret(m_file.readChunk(buf, 8));
            return Buf_Size{buf, 0};
        }

        case DDT_Int:
        case DDT_Real:
        case DDT_Money:
        {
            condret(m_file.readChunk(buf, 4));
            return Buf_Size{buf, 0};
        }

        case DDT_SmallInt:
        {
            condret(m_file.readChunk(buf, 2));
            return Buf_Size{buf, 0};
        }

        case DDT_Boolean:
        {
            condret(m_file.readChunk(buf, 1));
            return Buf_Size{buf, 0};
        }

        case DDT_Bit:
        {
            DBMOD clean = mod;
            condret(m_file.readChunk(buf, CerberusUtils::qceil(clean, 8)));
            return Buf_Size{buf, 0};
        }

        case DDT_Char:
        {
            DBMOD clean = mod;
            condret(m_file.readChunk(buf, clean));
            return Buf_Size{buf, 0};
        }

        case DDT_VarBit:
        {
            DBMOD clean = mod;
            LSIZE bytes = CerberusUtils::reqBytes(clean);  // required bytes to represent the number

            condret(m_file.readChunk(buf, bytes));  // read field header (actual number of bits)
            LSIZE actualBits = 0;
            buf.copyTo(&actualBits, sizeof(actualBits));

            bytes = CerberusUtils::qceil(actualBits, 8);  // allocated bytes

            condret(m_file.readChunk(buf, bytes));

            return Buf_Size{buf, actualBits};
        }

        case DDT_VarChar:
        {
            DBMOD clean = mod;
            LSIZE bytes = CerberusUtils::reqBytes(clean);  // required bytes to represent the number

            condret(m_file.readChunk(buf, bytes));  // read field header (actual number of bytes)
            LSIZE actualBytes = 0;
            buf.copyTo(&actualBytes, sizeof(actualBytes));

            condret(m_file.readChunk(buf, actualBytes));

            return Buf_Size{buf, actualBytes};
        }

        default:
            break;
    }

    return {OR_WrongArgument, "unknown typeid in _parseFieldRaw()"};
}
//=============================================================================
OpResData<DBCell> FilesystemDB::_parseField(DBDataType type, DBMOD mod)
{
    auto buf = _parseFieldRaw(type, mod);
    condret_str(buf, "readchunk");
    return DBCell(buf.value.buf);
}
//=============================================================================
OpRes FilesystemDB::_skipTable(Table& tab, LSIZE* endPos)
{
    ByteBuffer buf;
    condret_str(m_file.seek(tab.buffer - 8), "seek size");
    condret_str(m_file.readChunk(buf, 8), "error while parsing table size");
    LSIZE size = 0;  // number of rows
    buf.copyTo(&size);

    condret_str(m_file.seek(tab.buffer), "seek buffer");

    for (LSIZE i = 0; i < size; i++)
    {
        for (SIZE j = 0; j < tab.proto.size(); j++)
        {
            auto raw = _parseFieldRaw(tab.proto[j].type(), tab.proto[j].mod());
            condret(raw);
        }
    }

    auto cur = m_file.getCursor();
    condret(cur);
    if (endPos) *endPos = cur.value;
    return OR_OK;
}
//=============================================================================
OpResData<DBTableBlock> FilesystemDB::_getTable(Table& tab)
{
    DBTableBlock block(tab.proto);

    ByteBuffer buf;
    condret_str(m_file.seek(tab.buffer - 8), "seek size");
    condret_str(m_file.readChunk(buf, 8), "error while parsing table size");
    LSIZE size = 0;  // number of rows
    buf.copyTo(&size);

    condret_str(m_file.seek(tab.buffer), "seek buffer");

    DBRow row;
    bool dupFound = false;
    int pkIndex   = tab.proto.primaryKeyIndex();
    std::unordered_set<std::string> seenKeys;

    for (LSIZE i = 0; i < size; i++)  // for each row
    {
        row.clear();

        for (SIZE j = 0; j < tab.proto.size(); j++)  // for each column
        {
            auto cell = _parseField(tab.proto[j].type(), tab.proto[j].mod());
            condret(cell);

            row.append(cell.value);
        }

        block.append(row);

        if (pkIndex >= 0)
        {
            std::string key = _cellToString(row[pkIndex], tab.proto[pkIndex].type());
            if (!seenKeys.insert(key).second) dupFound = true;
        }
    }

    tab.hasDupKey = dupFound;

    OpResData<DBTableBlock> res(block);
    if (dupFound) res.addOptional(OR_DuplicateKey);
    return res;
}
//=============================================================================
OpRes FilesystemDB::_insertBlock(Table* tab, const DBTableBlock& block)
{
    if (!block.verify(tab->proto)) return {OR_WrongType, "the block cannot be verified"};
    if (tab->hasDupKey) return OR_DuplicateKey;

    for (auto&& row : block)
        if (row.size() != tab->proto.size())
            return {OR_DBMissingColumns, "row size does not match table prototype"};

    int pkIndex = tab->proto.primaryKeyIndex();
    if (pkIndex >= 0)
    {
        auto existing = _getTable(*tab);
        if (existing.fail()) return existing;
        if (existing.hasOptional(OR_DuplicateKey)) return OR_DuplicateKey;

        std::unordered_set<std::string> keys;
        for (auto&& r : existing.value)
            keys.insert(_cellToString(r[pkIndex], tab->proto[pkIndex].type()));

        for (auto&& r : block)
        {
            std::string k = _cellToString(r[pkIndex], tab->proto[pkIndex].type());
            if (!keys.insert(k).second) return OR_DuplicateKey;
        }
    }

    // position the cursor at the beginning of the buffer
    condret_str(m_file.seek(tab->buffer), "seek");

    ByteBuffer buf = block.serialize(tab->proto);  // generate the block of data

    condret_str(m_file.insert(buf), "insert");

    // update the line count field
    condret_str(m_file.read(buf, tab->buffer - 8, 8), "read");
    LSIZE rowscount = 0;
    buf.copyTo(&rowscount);
    rowscount += block.size();

    condret_str(m_file.seek(tab->buffer - 8), "seek2");
    condret_str(m_file.write({&rowscount, 8}), "write");

    return OR_OK;
}
//=============================================================================
FilesystemDB::FilesystemDB()
    : m_file(FOM_ReadWrite)
{
}
//=============================================================================
OpRes FilesystemDB::init(const string& parameters)
{
    // Reset any previous state to avoid altering the open mode of an open file
    if (m_file.isOpen()) m_file.close();

    m_file.path(parameters);
    m_file.setOpenMode(FOM_ReadWrite);

    auto res = m_file.open();
    if (res.fail())
    {
        // Create/truncate the file, then reopen in normal read/write mode
        m_file.setOpenMode(FOM_ReadWriteTrunc);
        condret(m_file.open());
        m_file.close();
        m_file.setOpenMode(FOM_ReadWrite);
        res = m_file.open();
    }

    condret(res);

    return _load();
}
//=============================================================================
void FilesystemDB::deinit() { m_file.close(); }
//=============================================================================
bool FilesystemDB::ready() const { return m_file.isOpen(); }
//=============================================================================
OpRes FilesystemDB::command(const string& query)
{
    throw cUsageErrorExc("cannot give a command to the \"Filesystem\" DB backend");
}
//=============================================================================
FilesystemDB::ParsedQuery FilesystemDB::_splitQuery(const std::string& query) const
{
    ParsedQuery pq;
    auto first = query.find(':');
    if (first == std::string::npos) throw cIllegalArgExc("bad query format");

    pq.table = query.substr(0, first);

    auto colPos = query.rfind(":[");
    if (colPos == std::string::npos)
    {
        pq.pattern = query.substr(first + 1);
        if (!pq.pattern.empty() && pq.pattern.back() == ':') pq.pattern.pop_back();
    }
    else
    {
        pq.pattern = query.substr(first + 1, colPos - first - 1);
        pq.columns = query.substr(colPos + 1);
    }

    return pq;
}
//=============================================================================
int FilesystemDB::_columnIndex(const DBTableProto& proto, const std::string& name) const
{
    for (size_t i = 0; i < proto.size(); ++i)
        if (proto[i].name() == name) return static_cast<int>(i);
    return -1;
}
//=============================================================================
std::vector<int> FilesystemDB::_parseColumns(const std::vector<std::string>& columns, const DBTableProto& proto,
                                             DBTableProto& outProto) const
{
    std::vector<int> selIndexes;

    if (columns.empty())
    {
        selIndexes.resize(proto.size());
        std::iota(selIndexes.begin(), selIndexes.end(), 0);
        outProto = proto;
        return selIndexes;
    }

    outProto.clear();
    outProto.setName(proto.name());
    for (const auto& colName : columns)
    {
        std::string name = CerberusUtils::removeBlank_copy(colName);
        int idx          = _columnIndex(proto, name);
        if (idx < 0) throw cIllegalArgExc("column not found");
        selIndexes.push_back(idx);
        outProto.add(proto[idx].name(), proto[idx].type(), proto[idx].mod(), proto[idx].flags(),
                     proto[idx].defaultValue());
    }

    return selIndexes;
}
//=============================================================================
std::string FilesystemDB::_cellToString(const DBCell& cell, DBDataType t) const
{
    switch (t)
    {
        case DDT_BigInt:
        case DDT_Int:
        case DDT_SmallInt:
            return std::to_string(cell.toInt());
        case DDT_Double:
        case DDT_Real:
        case DDT_Money:
            return std::to_string((double)cell.toReal());
        case DDT_Boolean:
            return cell.toBool() ? "true" : "false";
        case DDT_Char:
        case DDT_VarChar:
            return cell.raw().toString();
        case DDT_Bit:
        case DDT_VarBit:
            return cell.raw().toHex();
        default:
            return cell.raw().toString();
    }
}
//=============================================================================
bool FilesystemDB::_matchRow(const DBQueryCondition& cond, int colIndex, const DBRow& row,
                             const DBTableProto& proto) const
{
    const DBCell& cell  = row[colIndex];
    DBDataType colType = proto[colIndex].type();

    switch (cond.type)
    {
        case DBQC_Exact: {
            const std::string& exact = cond.exactValue;
            if (colType == DDT_Boolean)
                return (exact == "true" && cell.toBool()) || (exact == "false" && !cell.toBool());
            if (colType == DDT_Char || colType == DDT_VarChar)
                return _cellToString(cell, colType) == exact;
            if (colType == DDT_Bit || colType == DDT_VarBit)
                return cell.raw().toHex() == exact;

            long double v = (colType == DDT_Int || colType == DDT_SmallInt || colType == DDT_BigInt)
                                ? (long double)cell.toInt()
                                : cell.toReal();
            return v == std::stold(exact);
        }
        case DBQC_Range: {
            long double v = 0.0;
            if (colType == DDT_Char || colType == DDT_VarChar)
            {
                std::string s = _cellToString(cell, colType);
                if (cond.min && s < std::to_string(*cond.min)) return false;
                if (cond.max && s > std::to_string(*cond.max)) return false;
                return true;
            }
            else if (colType == DDT_Int || colType == DDT_SmallInt || colType == DDT_BigInt)
                v = (long double)cell.toInt();
            else
                v = cell.toReal();

            std::optional<long double> min = cond.min;
            std::optional<long double> max = cond.max;
            if (cond.invertOrder && min.has_value() && max.has_value())
            {
                long double tmp = min.value();
                min             = max;
                max             = tmp;
            }
            if (min && v < *min) return false;
            if (max && v > *max) return false;
            return true;
        }
        case DBQC_Regex: {
            std::string s = _cellToString(cell, colType);
            boost::regex rx(cond.regexPattern, boost::regex::perl | boost::regex::optimize);
            return boost::regex_search(s, rx);
        }
        case DBQC_None:
        default:
            break;
        }

    return false;
}
//=============================================================================
OpResData<DBTableBlock> FilesystemDB::queryBlock(const string& query)
{
    auto q = DBQuery::fromString(query);
    if (q.fail()) return q;
    return queryBlock(q.value);
}
//=============================================================================
OpResData<DBTableBlock> FilesystemDB::queryBlock(const DBQuery& query)
{
    if (!m_file.isOpen()) return OR_BadConditions;

    auto valid = query.validate();
    if (valid.fail()) return valid;

    auto tabRes = querytable(query.table());
    if (tabRes.fail()) return tabRes;
    DBTableBlock table = tabRes.value;
    const DBTableProto& proto = table.prototype();

    int colIndex = _columnIndex(proto, query.condition().column);
    if (colIndex < 0) return {OR_NotFound, "column not found: " + query.condition().column};

    std::vector<int> selIndexes;
    DBTableProto outProto = proto;
    try
    {
        selIndexes = _parseColumns(query.columns(), proto, outProto);
    }
    catch (const std::exception& e)
    {
        return {OR_WrongArgument, e.what()};
    }

    DBTableBlock result(outProto);
    std::vector<DBRow> matches;
    matches.reserve(table.size());

    for (auto&& row : table)
    {
        bool match = false;
        try
        {
            match = _matchRow(query.condition(), colIndex, row, proto);
        }
        catch (const std::exception& e)
        {
            return {OR_WrongArgument, "invalid query condition", e.what()};
        }
        if (!match) continue;

        DBRow outRow;
        outRow.bindPrototype(&outProto);
        for (auto idx : selIndexes) outRow.append(row[static_cast<size_t>(idx)]);
        matches.push_back(outRow);
    }

    if (query.condition().invertOrder)
    {
        for (auto it = matches.rbegin(); it != matches.rend(); ++it)
        {
            auto ar = result.append(*it);
            if (ar.fail()) return ar;
        }
    }
    else
    {
        for (auto&& r : matches)
        {
            auto ar = result.append(r);
            if (ar.fail()) return ar;
        }
    }

    return result;
}
//=============================================================================
OpResData<DBTableProto> FilesystemDB::queryPrototype(const string& tableName)
{
    if (!m_file.isOpen()) return OR_BadConditions;

    auto id = HASH_FUNC(tableName);

    for (auto&& el : m_tables)
        if (el.tableID == id) return el.proto;

    return OR_NotFound;
}
//=============================================================================
OpRes FilesystemDB::createTable(const DBTableProto& prototype)
{
    if (!m_file.isOpen()) return OR_BadConditions;

    int keyCount = 0;
    for (auto&& c : prototype)
        if (c.isPrimary()) ++keyCount;
    if (keyCount != 1) return {OR_WrongArgument, "table must contain exactly one primary key column"};

    for (auto&& c : prototype)
    {
        try
        {
            auto def = c.defaultValue().serialize(c.type(), c.mod());
            if (c.type() == DDT_Char && def.size() > c.mod())
                return {OR_WrongType, "default value too large for char column: " + c.name()};
            if (c.type() == DDT_VarChar && c.mod() > 0)
            {
                LSIZE hdr = CerberusUtils::reqBytes(c.mod());
                if (def.size() > hdr + c.mod())
                    return {OR_WrongType, "default value too large for varchar column: " + c.name()};
            }
            if (c.type() == DDT_Bit && def.size() > CerberusUtils::qceil(c.mod(), 8))
                return {OR_WrongType, "default value too large for bit column: " + c.name()};
            if (c.type() == DDT_VarBit && c.mod() > 0)
            {
                LSIZE hdr = CerberusUtils::reqBytes(c.mod());
                if (def.size() > hdr + CerberusUtils::qceil(c.mod(), 8))
                    return {OR_WrongType, "default value too large for varbit column: " + c.name()};
            }
        }
        catch (const std::exception& e)
        {
            return {OR_WrongType, "invalid default value for column " + c.name(), e.what()};
        }
    }

    // verify if the table already exists

    HASH32 id = HASH_FUNC(prototype.name());

    for (auto&& el : m_tables)
        if (el.tableID == id) return OR_AlreadyPresent;

    Table tab(id, prototype, 0, 0);
    condret(_buildHeader(prototype, tab));
    m_tables.push_back(tab);

    return OR_OK;
}
//=============================================================================
OpRes FilesystemDB::insertBlock(const DBTableBlock& block)
{
    if (!m_file.isOpen()) return OR_BadConditions;

    HASH32 id = HASH_FUNC(block.prototype().name());

    for (auto&& el : m_tables)
        if (el.tableID == id) return _insertBlock(&el, block);

    return OR_NotFound;
}
//=============================================================================
OpRes FilesystemDB::updateBlock(const DBTableBlock& block, UpdatePolicy policy)
{
    if (!m_file.isOpen()) return OR_BadConditions;
    if (!block.structured()) return OR_WrongArgument;
    if (block.empty()) return OR_OK;

    HASH32 id = HASH_FUNC(block.prototype().name());
    Table* tab = nullptr;
    for (auto& t : m_tables)
        if (t.tableID == id)
        {
            tab = &t;
            break;
        }

    if (tab == nullptr) return OR_NotFound;
    if (tab->hasDupKey) return OR_DuplicateKey;

    const DBTableProto& tproto = tab->proto;
    const DBTableProto& bproto = block.prototype();

    int tPk = tproto.primaryKeyIndex();
    if (tPk < 0) return OR_WrongArgument;

    std::vector<int> bToT(bproto.size(), -1);
    int bPk = -1;

    for (size_t i = 0; i < bproto.size(); ++i)
    {
        const auto& bc = bproto[i];
        int tIdx       = _columnIndex(tproto, bc.name());
        if (tIdx < 0) return OR_WrongArgument;

        const auto& tc = tproto[tIdx];
        DBMOD bMod     = bc.mod();
        DBMOD tMod     = tc.mod();
        if (bc.type() != tc.type() || bMod != tMod)
            return {OR_WrongType, "column type mismatch on " + bc.name()};

        bToT[i] = tIdx;
        if (tIdx == tPk) bPk = static_cast<int>(i);
    }

    if (bPk < 0) return {OR_WrongArgument, "missing primary key column in update block"};
    if (!block.verify(bproto)) return {OR_WrongType, "block verification failed"};

    std::unordered_set<std::string> keys;
    for (auto&& r : block)
    {
        std::string k = _cellToString(r[bPk], tproto[tPk].type());
        if (!keys.insert(k).second) return {OR_DuplicateKey, "duplicate key in update block", k};
    }

    auto current = _getTable(*tab);
    condret_str(current, "get table");
    if (current.hasOptional(OR_DuplicateKey)) return OR_DuplicateKey;

    std::unordered_map<std::string, const DBRow*> updateRows;
    updateRows.reserve(block.size());
    for (auto&& r : block)
    {
        std::string k = _cellToString(r[bPk], tproto[tPk].type());
        updateRows[k] = &r;
    }

    std::unordered_set<std::string> existingKeys;
    existingKeys.reserve(current.value.size());
    for (auto&& r : current.value) existingKeys.insert(_cellToString(r[tPk], tproto[tPk].type()));

    std::unordered_set<std::string> keysToInsert;
    std::unordered_set<std::string> keysToUpdate;
    for (auto&& k : keys)
    {
        if (existingKeys.find(k) == existingKeys.end())
            keysToInsert.insert(k);
        else
            keysToUpdate.insert(k);
    }

    if (policy == UP_UpdateOnly && !keysToInsert.empty())
        return {OR_NotFound, "update-only policy rejected missing keys",
                "count=" + std::to_string(keysToInsert.size())};

    if (policy == UP_InsertOnly && !keysToUpdate.empty())
        return {OR_DuplicateKey, "insert-only policy rejected existing keys",
                "count=" + std::to_string(keysToUpdate.size())};

    ByteBuffer buf;
    condret_str(m_file.read(buf, tab->buffer - 8, 8), "read rowscount");
    LSIZE rows = 0;
    buf.copyTo(&rows);
    condret_str(m_file.seek(tab->buffer), "seek buffer");

    std::unordered_set<std::string> pending(keysToInsert.begin(), keysToInsert.end());
    bool needRewrite = false;

    if (policy != UP_InsertOnly && !keysToUpdate.empty())
    {
        for (LSIZE r = 0; r < rows; ++r)
        {
            std::vector<LSIZE> starts(tproto.size(), 0);
            std::vector<LSIZE> lens(tproto.size(), 0);
            const DBRow* updRow = nullptr;

            for (size_t c = 0; c < tproto.size(); ++c)
            {
                auto cur = m_file.getCursor();
                condret(cur);
                starts[c] = cur.value;

                auto raw = _parseFieldRaw(tproto[c].type(), tproto[c].mod());
                condret(raw);

                auto after = m_file.getCursor();
                condret(after);
                lens[c] = after.value - starts[c];

                if (c == static_cast<size_t>(tPk))
                {
                    DBCell cell(raw.value.buf);
                    std::string key = _cellToString(cell, tproto[c].type());
                    auto it         = updateRows.find(key);
                    if (it != updateRows.end())
                    {
                        updRow = it->second;
                        pending.erase(key);
                    }
                }
            }

            if (updRow != nullptr)
            {
                for (size_t bi = 0; bi < bproto.size(); ++bi)
                {
                    int tIdx = bToT[bi];
                    if (tIdx == tPk) continue;

                    DBDataType tp  = tproto[tIdx].type();
                    DBMOD mod      = tproto[tIdx].mod();
                    ByteBuffer ser = (*updRow)[bi].serialize(tp, mod);
                    LSIZE newLen   = ser.size();
                    LSIZE oldLen   = lens[tIdx];

                    if (tp == DDT_Char || tp == DDT_Bit)
                    {
                        if (newLen > oldLen) return {OR_DBFieldSizeMismatch, "field too large for fixed column"};
                        if (newLen < oldLen) ser.append(ByteBuffer(oldLen - newLen, 0));
                        condret_str(m_file.seek(starts[tIdx]), "seek update");
                        condret_str(m_file.write(ser), "write update");
                    }
                    else if (tp == DDT_VarChar || tp == DDT_VarBit)
                    {
                        DBMOD clean = mod;
                        LSIZE hdr   = CerberusUtils::reqBytes(clean);
                        LSIZE maxPayload =
                            (tp == DDT_VarBit) ? CerberusUtils::qceil(clean, 8) : static_cast<LSIZE>(clean);
                        if (newLen > hdr + maxPayload)
                            return {OR_DBFieldSizeMismatch, "field exceeds column modifier"};
                        if (newLen != oldLen)
                            needRewrite = true;
                        else
                        {
                            condret_str(m_file.seek(starts[tIdx]), "seek update");
                            condret_str(m_file.write(ser), "write update");
                        }
                    }
                    else
                    {
                        if (newLen != oldLen)
                            return {OR_DBFieldSizeMismatch, "fixed-size field serialization mismatch"};
                        condret_str(m_file.seek(starts[tIdx]), "seek update");
                        condret_str(m_file.write(ser), "write update");
                    }
                }
            }
        }
    }

    if (needRewrite)
    {
        DBTableBlock rebuilt = current.value;

        if (policy != UP_InsertOnly)
        {
            for (auto& r : rebuilt)
            {
                std::string key = _cellToString(r[tPk], tproto[tPk].type());
                auto it         = updateRows.find(key);
                if (it == updateRows.end()) continue;
                const DBRow* ur = it->second;
                for (size_t bi = 0; bi < bproto.size(); ++bi)
                {
                    int tIdx = bToT[bi];
                    if (tIdx < 0 || tIdx == tPk) continue;
                    r[static_cast<size_t>(tIdx)] = (*ur)[bi];
                }
            }
        }

        if ((policy == UP_InsertOnly || policy == UP_UpdateInsert) && !keysToInsert.empty())
        {
            for (auto& kv : updateRows)
            {
                if (keysToInsert.find(kv.first) == keysToInsert.end()) continue;
                DBRow nr;
                condret(buildInsertRowFromUpdate(kv.second, tproto, bproto, bToT, nr));
                condret(rebuilt.append(nr));
            }
        }

        std::string name = tab->proto.name();
        condret(dropTable(name));
        condret(createTable(tproto));
        condret(insertBlock(rebuilt));
        return OR_OK;
    }

    if ((policy == UP_InsertOnly || policy == UP_UpdateInsert) && !keysToInsert.empty())
    {
        DBTableBlock toInsert(tproto);
        for (auto& kv : updateRows)
        {
            if (keysToInsert.find(kv.first) == keysToInsert.end()) continue;
            DBRow nr;
            condret(buildInsertRowFromUpdate(kv.second, tproto, bproto, bToT, nr));
            condret(toInsert.append(nr));
        }

        if (!toInsert.empty()) condret(_insertBlock(tab, toInsert));
    }

    return OR_OK;
}
//=============================================================================
OpRes FilesystemDB::renameColumn(const std::string& table, const std::string& oldName,
                                 const std::string& newName)
{
    if (!m_file.isOpen()) return OR_BadConditions;

    auto curr = querytable(table);
    if (curr.fail()) return curr;

    DBTableProto proto = curr.value.prototype();
    auto rr = proto.renameColumn(oldName, newName);
    if (rr.fail()) return rr;

    condret(dropTable(table));
    condret(createTable(proto));
    condret(insertBlock(curr.value));

    return OR_OK;
}
//=============================================================================
OpRes FilesystemDB::dropTable(const string& table)
{
    if (!m_file.isOpen()) return OR_BadConditions;

    HASH32 id = HASH_FUNC(table);
    auto it = m_tables.end();
    for (auto itr = m_tables.begin(); itr != m_tables.end(); ++itr)
    {
        if (itr->tableID == id)
        {
            it = itr;
            break;
        }
    }
    if (it == m_tables.end()) return OR_NotFound;

    LSIZE start = it->header - 2;  // include the leading "//"
    LSIZE end   = start;
    condret_str(_skipTable(*it, &end), "skip table");
    LSIZE span = end - start;

    auto res = m_file.erase(start, span);
    condret(res);

    // reload metadata
    return _load();
}
//=============================================================================
OpResData<DBTableBlock> FilesystemDB::querytable(const string& tableName)
{
    if (!m_file.isOpen()) return OR_BadConditions;

    auto id = HASH_FUNC(tableName);

    for (auto&& el : m_tables)
        if (el.tableID == id) return _getTable(el);

    return OR_NotFound;
}
//=============================================================================
FilesystemDB::~FilesystemDB() { m_file.close(); }
//=============================================================================
