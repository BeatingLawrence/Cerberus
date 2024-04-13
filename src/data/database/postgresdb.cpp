#ifdef POSTGRESQL_SUPPORT
#include "postgresdb.h"

#include <pqxx/pqxx>

#include "../../core/cerberusutils.h"
#include "dbdata.h"

using namespace cerberus::db;
using namespace cerberus;

//=============================================================================
PostgresDB::PostgresDB()
    : m_connection(nullptr)
{
}
//=============================================================================
OpRes PostgresDB::init(const string &parameters)
{
    try
    {
        m_connection = new pqxx::connection(parameters.c_str());
    }
    catch (pqxx::failure e)
    {
        return OpRes(OR_Failure, "PostgreSQL database init error", e.what());
    }

    return OR_OK;
}
//=============================================================================
void PostgresDB::deinit()
{
    if (m_connection != nullptr)
    {
        delete m_connection;
        m_connection = nullptr;
    }
}
//=============================================================================
OpRes PostgresDB::command(const string &query)
{
    try
    {
        pqxx::work w(*m_connection);
        w.exec0(query);
        w.commit();
    }
    catch (pqxx::sql_error e)
    {
        OpRes r(OR_QueryFailure, "PostgreSQL query error", e.what());
        r.addInfo(e.sqlstate());
        return r;
    }
    catch (pqxx::failure e)
    {
        return {OR_DBFailure, "PostgreSQL DB error while procesing query", e.what()};
    }

    return OR_OK;
}
//=============================================================================
OpResData<DBTableBlock> PostgresDB::queryBlock(const string &query)
{
    DBTableBlock ret;

    try
    {
        pqxx::work w(*m_connection);
        pqxx::result res = w.exec(query);
        w.commit();

        for (auto &&row : res)
        {
            DBRow r;

            for (auto &&val : row)
            {
                r.append(val.c_str());
            }

            ret.append(r);
        }

        if (ret.empty()) return OR_NotFound;
    }
    catch (pqxx::sql_error e)
    {
        OpRes r(OR_QueryFailure, "PostgreSQL query error", e.what());
        r.addInfo(e.sqlstate());
        return r;
    }
    catch (pqxx::failure e)
    {
        return {OR_DBFailure, "PostgreSQL DB error while procesing query", e.what()};
    }

    return ret;
}
//=============================================================================
OpResData<DBTableProto> PostgresDB::queryPrototype(const string &tableName)
{
    if (tableName.empty()) return OR_WrongArgument;

    auto res = queryBlock(
        CerberusUtils::strPrint("SELECT a.attname as \"Column\", pg_catalog.format_type(a.atttypid, "
                                "a.atttypmod) as \"Datatype\", a.atttypmod as \"Mod\" "
                                "FROM pg_catalog.pg_attribute a "
                                "WHERE a.attnum > 0 "
                                "AND NOT a.attisdropped "
                                "AND a.attrelid = ( "
                                "SELECT c.oid "
                                "FROM pg_catalog.pg_class c "
                                "LEFT JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace "
                                "WHERE c.relname = '%s' "
                                "AND pg_catalog.pg_table_is_visible(c.oid));",
                                tableName.c_str()));

    if (res.fail()) return res;

    DBTableProto proto(tableName);

    for (int i = 0; i < res.value.size(); i++)
    {
        proto.add(res.value[i][0].raw(), CerberusUtils::toSQLDataType(res.value[i][1].raw()),
                  res.value[i][2].toInt());
    }

    return res;
}
//=============================================================================
PostgresDB::~PostgresDB() { deinit(); }
//=============================================================================
#endif
