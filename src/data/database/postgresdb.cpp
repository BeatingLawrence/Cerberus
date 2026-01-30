#ifdef POSTGRESQL_SUPPORT
#include "postgresdb.h"

#include <pqxx/pqxx>

#include "../../core/cerberusutils.h"
#include "dbdata.h"

using namespace crb::db;
using namespace crb;

//=============================================================================
void PostgresDB::_deinit()
{
    if (m_connection != nullptr)
    {
        delete m_connection;
        m_connection = nullptr;
    }

    m_ready = false;
}
//=============================================================================
PostgresDB::PostgresDB()
    : m_connection(nullptr)
{
}
//=============================================================================
OpRes PostgresDB::init(const string& parameters)
{
    if (m_ready) return OR_BadConditions;

    try
    {
        m_connection = new pqxx::connection(parameters.c_str());
    }
    catch (pqxx::failure e)
    {
        return OpRes(OR_Failure, "PostgreSQL database init error", e.what());
    }

    m_ready = true;
    return OR_OK;
}
//=============================================================================
void PostgresDB::deinit() { _deinit(); }
//=============================================================================
bool PostgresDB::ready() const {}  // IMPLEMENT THIS METHOD
//=============================================================================
OpRes PostgresDB::command(const string& query)
{
    if (!m_ready) return OR_BadConditions;

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
OpResData<DBTableBlock> PostgresDB::queryBlock(const string& query)
{
    if (!m_ready) return OR_BadConditions;

    DBTableBlock ret;

    try
    {
        pqxx::work w(*m_connection);
        pqxx::result res = w.exec(query);
        w.commit();

        for (auto&& row : res)
        {
            DBRow r;

            for (auto&& val : row)
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
OpResData<DBTableProto> PostgresDB::queryPrototype(const string& tableName)
{
    if (!m_ready) return OR_BadConditions;

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
OpRes PostgresDB::createTable(const DBTableProto& prototype)  // PURE SQL
{
    auto name = prototype.name();

    if (name.empty()) return OR_WrongArgument;

    auto res = queryPrototype(name);

    if (res.ok()) return OR_AlreadyPresent;

    if (res.res != OR_NotFound) return res;

    std::string cmd("CREATE TABLE ");
    cmd += prototype.name();
    cmd += " ( ";

    for (auto&& el : prototype)
    {
        auto type = el.type();

        if (type == DBDataType::SDT_Bit || type == DBDataType::SDT_VarBit || type == DBDataType::SDT_Char ||
            type == DBDataType::SDT_VarChar)  // check if the mod parameter should be printed
        {
            cmd +=
                CerberusUtils::strPrint("%s %s(%i), ", el.name().c_str(), el.typeString().c_str(), el.mod());
        }
        else
        {
            cmd += CerberusUtils::strPrint("%s %s, ", el.name().c_str(), el.typeString().c_str());
        }
    }

    cmd.pop_back();
    cmd.pop_back();  // remove space and comma
    cmd += ");";
    return command(cmd);
}
//=============================================================================
OpRes PostgresDB::insertBlock(const DBTableBlock& block)  // PURE SQL
{
    if (block.prototype().name().empty()) return OR_WrongArgument;

    std::string cmd("INSERT INTO ");
    cmd += block.prototype().name();
    cmd += " VALUES ";

    for (auto&& row : block)
    {
        cmd += '(';

        for (int i = 0; i < row.size(); i++)
        {
            auto type = block.prototype()[i].type();

            if (type == DBDataType::SDT_Char ||
                type == DBDataType::SDT_VarChar)  // check if value is a number or a string
            {
                cmd += '\'';
                cmd += row[i].raw();
                cmd += '\'';
            }
            else
            {
                cmd += row[i].raw();
            }

            cmd += ", ";
        }

        cmd.pop_back();  // remove space and comma
        cmd.pop_back();
        cmd += "), ";
    }

    cmd.pop_back();  // remove space and comma
    cmd.pop_back();
    cmd += ";";
    return command(cmd);
}
//=============================================================================
OpRes PostgresDB::dropTable(const string& table)  // PURE SQL
{
    return command(CerberusUtils::strPrint("DROP TABLE %s;", table.c_str()));
}
//=============================================================================
OpResData<DBTableBlock> PostgresDB::querytable(const string& tableName)  // PURE SQL
{
    auto proto = queryPrototype(tableName);

    if (proto.fail()) return proto;

    DBTableBlock ret(proto.value);

    auto q = queryBlock(CerberusUtils::strPrint("SELECT * FROM %s;", tableName.c_str()));

    if (q.fail()) return q;

    ret.assignRows(q.value);

    return ret;
}
//=============================================================================
PostgresDB::~PostgresDB() { _deinit(); }
//=============================================================================
#endif
