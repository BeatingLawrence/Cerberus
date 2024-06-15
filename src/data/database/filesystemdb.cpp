#include "filesystemdb.h"

#include "../../exception/exception.h"

using namespace cerberus::db;

/*  STRUCTURE:
 *
 *  //xyyyyyyyy[...][0]table_name[0]col1_name[0]col2_name[...][0]buffer
 *
 *  x=typeid
 *  y=bytes mod (MSB ... LSB)
 *
 *  the first 8 bytes of buffer constitute the entry number (number of table rows)
 */

//=============================================================================
cerberus::OpResData<string> FilesystemDB::_readStr()
{
    auto n = m_file.readUntil({1, 0});  // find [0] delimiter
    if (n.fail()) return n;
    return n.value.toString();
}
//=============================================================================
cerberus::DBMOD FilesystemDB::_getMod(const ByteBuffer &buf)
{
    if (buf.size() != 8) return 0;
    DBMOD ret = 0;
    ret |= ((DBMOD)buf[7]);
    ret |= ((DBMOD)buf[6]) << 8;
    ret |= ((DBMOD)buf[5]) << 16;
    ret |= ((DBMOD)buf[4]) << 24;
    ret |= ((DBMOD)buf[3]) << 32;
    ret |= ((DBMOD)buf[2]) << 40;
    ret |= ((DBMOD)buf[1]) << 48;
    ret |= ((DBMOD)buf[0]) << 56;
    return ret;
}
//=============================================================================
cerberus::OpRes FilesystemDB::_parseTable()
{
    ByteBuffer buf;
    OpRes r;

    while (true)
    {
        r = m_file.readChunk(buf, 1);
        if (r.fail()) return r;

        DBDataType type = (DBDataType)buf.at(0);

        if (type == 0) break;  // first [0] delimiter found

        r = m_file.readChunk(buf, 8);
        if (r.fail()) return {r, "error while parsing typeID"};

        DBMOD mod = _getMod(buf);

        m_tables.back().proto.add("", type, mod);
    }

    auto str = _readStr();
    if (str.fail()) return {str, "error while parsing table name"};
    m_tables.back().proto.setName(str.value);  // table name

    for (auto &&el : m_tables.back().proto)
    {
        str = _readStr();
        if (str.fail()) return {str, "error while parsing column name"};
        el.setName(str.value);
    }

    m_tables.back().buffer = m_file.getCursor().expect().value;

    return OR_OK;
}
//=============================================================================
cerberus::OpRes FilesystemDB::_load()
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

        auto parseRes = _parseTable();

        if (parseRes.fail()) return parseRes;
    }
}
//=============================================================================
cerberus::OpRes FilesystemDB::_buildHeader(const DBTableProto &proto)
{
    // xyyyyyyyy[...][0]table_name[0]col1_name[0]col2_name[...][0]buffer

    ByteBuffer buf;
    buf.appendString("//");

    for (auto &&el : proto)
    {
        buf.appendChar(el.type());
        DBMOD mod = el.mod();
        buf.append_4b(&mod);
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

    condret(m_file.seekToEOF());  // check this
    condret(m_file.write(buf));

    // finish this method
    //
    return OR_OK;
}
//=============================================================================
FilesystemDB::FilesystemDB()
    : m_file(FOM_ReadWrite)
{
}
//=============================================================================
cerberus::OpRes FilesystemDB::init(const string &parameters)
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
cerberus::OpRes FilesystemDB::command(const string &query)
{
    throw cUsageErrorExc("Could not give a command to the \"Filesystem\" database backend");
}
//=============================================================================
cerberus::OpResData<cerberus::DBTableBlock> FilesystemDB::queryBlock(const string &query)
{
    throw cImplMissExc("queryBlock is not implemented yet");
}
//=============================================================================
cerberus::OpResData<cerberus::DBTableProto> FilesystemDB::queryPrototype(const string &tableName)
{
    throw cImplMissExc("queryPrototype is not implemented yet");
}
//=============================================================================
cerberus::OpRes FilesystemDB::createTable(const DBTableProto &prototype)
{
    // verify if the table already exists

    HASH32 id = CerberusUtils::hash_fnv1a(prototype.name());

    for (auto &&el : m_tables)
        if (el.tableID == id) return OR_AlreadyPresent;

    return _buildHeader(prototype);
}
//=============================================================================
cerberus::OpRes FilesystemDB::insertBlock(const DBTableBlock &block)
{
    throw cImplMissExc("insertBlock is not implemented yet");
}
//=============================================================================
cerberus::OpRes FilesystemDB::dropTable(const string &table)
{
    throw cImplMissExc("dropTable is not implemented yet");
}
//=============================================================================
cerberus::OpResData<cerberus::DBTableBlock> FilesystemDB::querytable(const string &tableName)
{
    throw cImplMissExc("querytable is not implemented yet");
}
//=============================================================================
FilesystemDB::~FilesystemDB() { m_file.close(); }
//=============================================================================
