#include "filesystemdb.h"

#include "../../exception/exception.h"

using namespace cerberus::db;
using namespace cerberus;

/*  STRUCTURE:
 *
 *  //xyyyyyyyy[...][0]table_name[0]col1_name[0]col2_name[...][0]buffer
 *
 *  x=typeid
 *  y=bytes mod (MSB ... LSB)
 *
 *  the first 8 bytes of buffer constitute the entry number (number of table rows)
 */

#define HASH_FUNC(str) CerberusUtils::hash_fnv1a(str)

//=============================================================================
OpResData<string> FilesystemDB::_readStr()
{
    auto n = m_file.readUntil({1, 0});  // find [0] delimiter
    if (n.fail()) return n;
    return n.value.toString();
}
//=============================================================================
DBMOD FilesystemDB::_getMod(const ByteBuffer &buf)
{
    if (buf.size() != 8) throw cIllegalArgExc("_getMod() called with wrong args");
    DBMOD ret = 0;
    // ret |= ((DBMOD)buf[7]);
    // ret |= ((DBMOD)buf[6]) << 8;
    // ret |= ((DBMOD)buf[5]) << 16;
    // ret |= ((DBMOD)buf[4]) << 24;
    // ret |= ((DBMOD)buf[3]) << 32;
    // ret |= ((DBMOD)buf[2]) << 40;
    // ret |= ((DBMOD)buf[1]) << 48;
    // ret |= ((DBMOD)buf[0]) << 56;
    buf.copyTo(&ret);
    return ret;
}
//=============================================================================
OpRes FilesystemDB::_parseTable()
{
    ByteBuffer buf;
    Table &tab = m_tables.back();

    while (true)
    {
        condret(m_file.readChunk(buf, 1));

        DBDataType type = (DBDataType)buf.at(0);

        if (type == 0) break;  // first [0] delimiter found

        condret_str(m_file.readChunk(buf, 8), "error while parsing typeID");

        DBMOD mod = _getMod(buf);
        tab.proto.add("", type, mod);
    }

    auto str = _readStr();
    condret_str(str, "error while parsing table name");

    tab.proto.setName(str.value);  // table name
    tab.tableID = HASH_FUNC(str.value);

    for (auto &&el : tab.proto)
    {
        str = _readStr();
        condret_str(str, "error while parsing column name");
        el.setName(str.value);
    }

    tab.buffer = m_file.getCursor().expect().value;

    // advance the cursor. This line will be substituted with an equivalent skip funcion
    condret_str(_getTable(&tab), "error while getting table data");

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
        tab.header = res.value;
        m_tables.push_back(tab);

        condret(_parseTable());
    }
}
//=============================================================================
OpRes FilesystemDB::_buildHeader(const DBTableProto &proto)
{
    ByteBuffer buf;
    buf.appendString("//");

    for (auto &&el : proto)
    {
        buf.appendChar(el.type());
        DBMOD mod = el.mod();
        buf.append_8b(&mod);
    }

    buf.appendChar(0);
    buf.appendString(proto.name());
    buf.appendChar(0);

    for (auto &&el : proto)
    {
        buf.appendString(el.name());
        buf.appendChar(0);
    }

    uint64_t size = 0;
    buf.append_8b(&size);  // reserve 8 bytes for the size field

    condret(m_file.writeExpand(buf));

    return OR_OK;
}
//=============================================================================
// LSIZE FilesystemDB::_byteAlign(LSIZE bits)
// {
//     if (bits % 8) bits += (8 - (bits % 8));
//     return bits;
// }
//=============================================================================
LSIZE FilesystemDB::_qceil(LSIZE x, LSIZE y) { return x / y + (x % y != 0); }
//=============================================================================
uint8_t FilesystemDB::_reqbytes(LSIZE num)
{
    if (!num) return 1;

    LSIZE mask = (LSIZE)0xFF << 56;

    for (uint8_t i = 8; i != 0; i--)
    {
        if (num & mask) return i;
        mask >>= 8;
    }

    return 1;
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
            return Buf_Size(buf, 0);
        }

        case DDT_Int:
        case DDT_Real:
        case DDT_Money:
        {
            condret(m_file.readChunk(buf, 4));
            return Buf_Size(buf, 0);
        }

        case DDT_SmallInt:
        {
            condret(m_file.readChunk(buf, 2));
            return Buf_Size(buf, 0);
        }

        case DDT_Boolean:
        {
            condret(m_file.readChunk(buf, 1));
            return Buf_Size(buf, 0);
        }

        case DDT_Bit:
        {
            condret(m_file.readChunk(buf, _qceil(mod, 8)));
            return Buf_Size(buf, 0);
        }

        case DDT_Char:
        {
            condret(m_file.readChunk(buf, mod));
            return Buf_Size(buf, 0);
        }

        case DDT_VarBit:
        {
            LSIZE bytes = _reqbytes(mod);  // required bytes to represent the number

            condret(m_file.readChunk(buf, bytes));  // read field header (actual number of bits)
            LSIZE actualBits = 0;
            buf.copyTo(&actualBits, sizeof(actualBits));

            bytes = _qceil(actualBits, 8);  // allocated bytes

            condret(m_file.readChunk(buf, bytes));

            return Buf_Size(buf, actualBits);
        }

        case DDT_VarChar:
        {
            LSIZE bytes = _reqbytes(mod);  // required bytes to represent the number

            condret(m_file.readChunk(buf, bytes));  // read field header (actual number of bytes)
            LSIZE actualBytes = 0;
            buf.copyTo(&actualBytes, sizeof(actualBytes));

            condret(m_file.readChunk(buf, actualBytes));

            return Buf_Size(buf, actualBytes);
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
    condret(buf);
    return DBCell(buf.value.buf, buf.value.size);
}
//=============================================================================
OpResData<DBTableBlock> FilesystemDB::_getTable(Table *tab)
{
    if (!tab) return OR_WrongData;

    condret(m_file.seek(tab->buffer));

    DBTableBlock block(tab->proto);

    ByteBuffer buf;
    condret_str(m_file.readChunk(buf, 8), "error while parsing table size");
    LSIZE size;  // number of rows
    buf.copyTo(&size);

    DBRow row;

    for (LSIZE i = 0; i < size; i++)  // for each row
    {
        row.clear();

        for (SIZE j = 0; j < tab->proto.size(); j++)  // for each column
        {
            auto cell = _parseField(tab->proto[j].type(), tab->proto[j].mod());
            condret(cell);

            row.append(cell.value);
        }

        block.append(row);
    }

    return block;
}
//=============================================================================
FilesystemDB::FilesystemDB()
    : m_file(FOM_ReadWrite)
{
}
//=============================================================================
OpRes FilesystemDB::init(const string &parameters)
{
    if (m_file.isOpen()) return OR_BadConditions;

    m_file.path(parameters);

    auto res = m_file.open();

    if (res.fail()) return res;

    return _load();
}
//=============================================================================
void FilesystemDB::deinit() { m_file.close(); }
//=============================================================================
OpRes FilesystemDB::command(const string &query)
{
    throw cUsageErrorExc("Could not give a command to the \"Filesystem\" database backend");
}
//=============================================================================
OpResData<DBTableBlock> FilesystemDB::queryBlock(const string &query)
{
    throw cImplMissExc("queryBlock is not implemented yet");
}
//=============================================================================
OpResData<DBTableProto> FilesystemDB::queryPrototype(const string &tableName)
{
    throw cImplMissExc("queryPrototype is not implemented yet");
}
//=============================================================================
OpRes FilesystemDB::createTable(const DBTableProto &prototype)
{
    // verify if the table already exists

    HASH32 id = HASH_FUNC(prototype.name());

    for (auto &&el : m_tables)
        if (el.tableID == id) return OR_AlreadyPresent;

    LSIZE h = m_file.size().expect().value;
    auto r  = _buildHeader(prototype);
    LSIZE b = m_file.size().expect().value - 5;  // back <sizebytes(4)> EOF byte

    m_tables.push_back({id, prototype, h, b});

    return OR_OK;
}
//=============================================================================
OpRes FilesystemDB::insertBlock(const DBTableBlock &block)
{
    throw cImplMissExc("insertBlock is not implemented yet");
}
//=============================================================================
OpRes FilesystemDB::dropTable(const string &table) { throw cImplMissExc("dropTable is not implemented yet"); }
//=============================================================================
OpResData<DBTableBlock> FilesystemDB::querytable(const string &tableName)
{
    auto id = HASH_FUNC(tableName);

    for (auto &&el : m_tables)
        if (el.tableID == id) return _getTable(&el);

    return OR_NotFound;
}
//=============================================================================
FilesystemDB::~FilesystemDB() { m_file.close(); }
//=============================================================================
