#include "filesystemdb.h"

#include "../../exception/exception.h"

using namespace cerberus::db;

//=============================================================================
cerberus::OpRes FilesystemDB::_load()
{
    if (!m_file.isOpen()) return OR_BadConditions;

    m_tables.clear();
    m_file.resetCursor();

    char c = 0;
    while (c != '/')
    {
        c = m_file.read()
    }
}
//=============================================================================
FilesystemDB::FilesystemDB()
    : m_file(FOM_ReadWrite, true)
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
cerberus::OpRes FilesystemDB::createTable(DBTableProto &prototype)
{
    throw cImplMissExc("createTable is not implemented yet");
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
