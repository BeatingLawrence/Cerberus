#include "filesystemdb.h"

#include "../../exception/exception.h"

using namespace cerberus::db;

//=============================================================================
FilesystemDB::FilesystemDB() {}
//=============================================================================
cerberus::OpRes FilesystemDB::init(const string &parameters)
{
    if (m_file.isOpen()) return OR_BadConditions;

    m_file.path(parameters);
    return m_file.open();
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
FilesystemDB::~FilesystemDB() { m_file.close(); }
//=============================================================================
