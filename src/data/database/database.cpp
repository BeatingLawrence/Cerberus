#include "database.h"

#include "../../cerberus.h"  // IWYU pragma: export
#include "filesystemdb.h"
#include "postgresdb.h"

using namespace cerberus;

//=============================================================================
Database::Database(DBBackend backend)
    : m_ready(false),
      m_db(nullptr)
{
    switch (backend)
    {
#ifdef POSTGRESQL_SUPPORT
        case DBB_PostgreSQL:
            m_db = new db::PostgresDB;
            break;

#endif
        case DBB_Filesystem:
            m_db = new db::FilesystemDB;
            break;

        default:
            throw cImplMissExc("Unknown database backend");
    }
}
//=============================================================================
Database::~Database()
{
    if (m_db) delete m_db;
}
//=============================================================================
OpRes Database::init(const string &parameters)
{
    auto ret = m_db->init(parameters);
    if (ret.ok()) m_ready = true;
    return ret;
}
//=============================================================================
void Database::deinit()
{
    m_db->deinit();
    m_ready = false;
}
//=============================================================================
OpRes Database::command(const string &query) { return m_db->command(query); }
//=============================================================================
OpResData<DBTableBlock> Database::queryBlock(const string &query) { return m_db->queryBlock(query); }
//=============================================================================
OpResData<DBTableProto> Database::queryPrototype(const string &tableName)
{
    return m_db->queryPrototype(tableName);
}
//=============================================================================
