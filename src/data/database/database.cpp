#include "database.h"

#include "../../cerberus.h"  // IWYU pragma: export
#include "filesystemdb.h"
#include "postgresdb.h"

using namespace crb;

//=============================================================================
Database::Database(DBBackend backend)
    : m_db(nullptr)
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
            throw cImplMissExc("Unknown database backend [or excluded from build]");
    }
}
//=============================================================================
Database::~Database()
{
    if (m_db) delete m_db;  // deinit is performed inside the backend implementation
}
//=============================================================================
