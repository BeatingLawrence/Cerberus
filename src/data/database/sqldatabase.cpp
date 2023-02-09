#include "sqldatabase.h"
#include <pqxx/connection>
#include <pqxx/result_iterator.hxx>
#include <pqxx/transaction>
#include "sqldata.h"
#include "../../cerberus.h"

using namespace cerberus::data::database;

//=============================================================================
SQLDatabase::SQLDatabase(const std::string& parameters) noexcept :
    m_connection(nullptr),
    m_failed(false),
    m_failureReason()
{
    try
    {
        m_connection = new pqxx::connection(parameters.c_str());
    }
    catch(pqxx::failure e)
    {
        m_failed = true;
        m_failureReason = e.what();
    }
}
//=============================================================================
SQLDatabase::~SQLDatabase()
{
    if(m_connection != nullptr)
    {
        delete m_connection;
    }
}
//=============================================================================
bool SQLDatabase::isFailed() const
{
    return m_failed;
}
//=============================================================================
std::string SQLDatabase::failureReason() const
{
    return m_failureReason;
}
//=============================================================================
SQLDatabase::OperationResult SQLDatabase::queryBlock(const std::string& query, SQLBlock& output)
{
    if(m_failed)    //return a failed result if connection is in a failed state
    {
        output.m_failed = true;
        output.m_failureReason = "Connection error: " + m_failureReason;
        return OperationResult::OR_DB_FAILURE;
    }

    output.clear();

    try
    {
        pqxx::work w(*m_connection);
        pqxx::result res = w.exec(query);
        w.commit();

        for(auto&& row : res)
        {
            SQLRow r;

            for(auto&& val : row)
            {
                r.append(val.c_str());
            }

            output.append(r);
        }

        if(output.empty())
        {
            return OperationResult::OR_NOT_FOUND;
        }
    }
    catch(pqxx::sql_error e)
    {
        logError(e.what());
        logError(e.sqlstate());
        output.m_failed = true; //return a failed result if query failed
        output.m_failureReason = e.what();
        return OperationResult::OR_QUERY_FAILURE;
    }
    catch(pqxx::failure e)
    {
        output.m_failed = true; //return a failed result if connection failed
        output.m_failureReason = "Database failure: " + std::string(e.what());
        m_failed = true;
        m_failureReason = e.what();
        return OperationResult::OR_DB_FAILURE;
    }

    return OperationResult::OR_OK;
}
//=============================================================================
SQLDatabase::OperationResult SQLDatabase::queryPrototype(SQLTablePrototype& prototype)
{
    if(prototype.m_name.empty())
    {
        logError("Queried the prototype of an empty-named table");
        return OperationResult::OR_QUERY_FAILURE;
    }

    SQLBlock block(prototype.m_name);
    auto res = queryBlock(core::CerberusUtils::strPrint("SELECT a.attname as \"Column\", pg_catalog.format_type(a.atttypid, a.atttypmod) as \"Datatype\", a.atttypmod as \"Mod\" "
                          "FROM pg_catalog.pg_attribute a "
                          "WHERE a.attnum > 0 "
                          "AND NOT a.attisdropped "
                          "AND a.attrelid = ( "
                          "SELECT c.oid "
                          "FROM pg_catalog.pg_class c "
                          "LEFT JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace "
                          "WHERE c.relname = '%s' "
                          "AND pg_catalog.pg_table_is_visible(c.oid));", prototype.m_name.c_str())
                          , block);

    if(res == OperationResult::OR_OK)
    {
        prototype.clear();

        for(int i = 0; i < block.size(); i++)
        {
            prototype.add(
                block[i][0].raw(),
                SQLTablePrototype::toSQLDataType(block[i][1].raw()),
                block[i][2].toInt());
        }
    }

    return res;
}
//=============================================================================
SQLDatabase::OperationResult SQLDatabase::command(const std::string& query)
{
    if(m_failed)
    {
        return OperationResult::OR_DB_FAILURE;
    }

    debug("executing: %s", query.c_str());

    try
    {
        pqxx::work w(*m_connection);
        w.exec0(query);
        w.commit();
    }
    catch(pqxx::sql_error e)
    {
        logError(e.what());
        logError(e.sqlstate());
        return OperationResult::OR_QUERY_FAILURE;
    }
    catch(pqxx::failure e)
    {
        m_failed = true;
        m_failureReason = e.what();
        return OperationResult::OR_DB_FAILURE;
    }

    return OperationResult::OR_OK;
}
//=============================================================================
SQLDatabase::OperationResult SQLDatabase::createTable(SQLTablePrototype& prototype)
{
    if(prototype.m_name.empty())
    {
        logError("Asked to create an empty-named table");
        return OperationResult::OR_QUERY_FAILURE;
    }

    SQLTablePrototype p(prototype.m_name);
    auto res = queryPrototype(p);

    if(res != OperationResult::OR_NOT_FOUND)
    {
        if(res == OperationResult::OR_OK)
        {
            logError("Asked to create an existing table");
            return OperationResult::OR_TABLE_ALREADY_PRESENT;
        }

        return res;
    }

    std::string cmd("CREATE TABLE ");
    cmd += prototype.m_name;
    cmd += " ( ";

    for(auto&& el : prototype.m_types)
    {
        SQLTablePrototype::SQLDataType type = std::get<1>(el);

        if(type == SQLTablePrototype::SDT_Bit
                || type == SQLTablePrototype::SDT_VarBit
                || type == SQLTablePrototype::SDT_Char
                || type == SQLTablePrototype::SDT_VarChar)  //check if the mod parameter should be printed
        {
            cmd += core::CerberusUtils::strPrint("%s %s(%i), ",
                                                 std::get<0>(el).c_str(),
                                                 SQLTablePrototype::fromSQLDataType(type).c_str(),
                                                 std::get<2>(el));
        }
        else
        {
            cmd += core::CerberusUtils::strPrint("%s %s, ",
                                                 std::get<0>(el).c_str(),
                                                 SQLTablePrototype::fromSQLDataType(type).c_str());
        }
    }

    cmd.pop_back();
    cmd.pop_back(); //remove space and comma
    cmd += ");";
    return command(cmd);
}
//=============================================================================
SQLDatabase::OperationResult SQLDatabase::insertBlock(const SQLBlock& block)
{
    if(block.m_prototype.m_name.empty())
    {
        logError("Asked to insert into an empty-named table");
        return OperationResult::OR_QUERY_FAILURE;
    }

    std::string cmd("INSERT INTO ");
    cmd += block.m_prototype.m_name;
    cmd += " VALUES ";

    for(auto&& row : block.m_rows)
    {
        cmd += '(';

        for(int i = 0; i < row.size(); i++)
        {
            SQLTablePrototype::SQLDataType type = std::get<1>(block.m_prototype.m_types[i]);

            if(type == SQLTablePrototype::SDT_Char || type == SQLTablePrototype::SDT_VarChar)  //check if value is a number or a string
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

        cmd.pop_back(); //remove space and comma
        cmd.pop_back();
        cmd += "), ";
    }

    cmd.pop_back(); //remove space and comma
    cmd.pop_back();
    cmd += ";";
    return command(cmd);
}
//=============================================================================
