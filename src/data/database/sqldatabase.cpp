#include "sqldatabase.h"

#include "../../cerberus.h"
#include "sqldata.h"

using namespace cerberus::data::database;

//=============================================================================
SQLDatabase::SQLDatabase(const std::string& parameters) noexcept
    : m_failed(false),
      m_failureReason()
{
    //connect to DB
}
//=============================================================================
SQLDatabase::~SQLDatabase()
{
    //de-init the DB
}
//=============================================================================
bool SQLDatabase::isFailed() const { return m_failed; }
//=============================================================================
std::string SQLDatabase::failureReason() const { return m_failureReason; }
//=============================================================================
cerberus::OperationResult SQLDatabase::queryBlock(const std::string& query, SQLBlock& output)
{
    //Query a single block
    return OR_Undefined;
}
//=============================================================================
cerberus::OperationResult SQLDatabase::queryPrototype(SQLTablePrototype& prototype)
{
    //Query a prototype
    return OR_Undefined;
}
//=============================================================================
cerberus::OperationResult SQLDatabase::command(const std::string& query)
{
    //Send a command
    return OR_Undefined;
}
//=============================================================================
cerberus::OperationResult SQLDatabase::createTable(SQLTablePrototype& prototype)
{
    if (prototype.name().empty())
    {
        logError("Asked to create an empty-named table");
        return OR_QueryFailure;
    }

    SQLTablePrototype p(prototype.name());
    auto res = queryPrototype(p);

    if (res != OR_NotFound)
    {
        if (res == OR_OK)
        {
            logError("Asked to create an existing table");
            return OR_TableAlreadyPresent;
        }

        return res;
    }

    std::string cmd("CREATE TABLE ");
    cmd += prototype.name();
    cmd += " ( ";

    for (auto&& el : prototype)
    {
        auto type = el.type();

        if (type == SQLTablePrototype::SDT_Bit || type == SQLTablePrototype::SDT_VarBit || type == SQLTablePrototype::SDT_Char ||
            type == SQLTablePrototype::SDT_VarChar)  // check if the mod parameter should be printed
        {
            cmd += core::CerberusUtils::strPrint("%s %s(%i), ", el.name().c_str(), el.typeString().c_str(), el.mod());
        }
        else
        {
            cmd += core::CerberusUtils::strPrint("%s %s, ", el.name().c_str(), el.typeString().c_str());
        }
    }

    cmd.pop_back();
    cmd.pop_back();  // remove space and comma
    cmd += ");";
    return command(cmd);
}
//=============================================================================
cerberus::OperationResult SQLDatabase::insertBlock(const SQLBlock& block)
{
    if (block.m_prototype.name().empty())
    {
        logError("Asked to insert into an empty-named table");
        return OR_QueryFailure;
    }

    std::string cmd("INSERT INTO ");
    cmd += block.m_prototype.name();
    cmd += " VALUES ";

    for (auto&& row : block.m_rows)
    {
        cmd += '(';

        for (int i = 0; i < row.size(); i++)
        {
            auto type = block.prototype()[i].type();

            if (type == SQLTablePrototype::SDT_Char || type == SQLTablePrototype::SDT_VarChar)  // check if value is a number or a string
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
cerberus::OperationResult SQLDatabase::dropTable(const std::string& table)
{
    return command(cerberus::core::CerberusUtils::strPrint("DROP TABLE %s;", table.c_str()));
}
//=============================================================================
cerberus::OperationResult SQLDatabase::querytable(const std::string& tableName, SQLBlock& output)
{
    SQLTablePrototype prototype(tableName);

    switch (queryPrototype(prototype).res)
    {
        case OR_QueryFailure:
            return OR_QueryFailure;
            break;

        case OR_DBFailure:
            return OR_DBFailure;
            break;

        case OR_NotFound:
            return OR_NotFound;
            break;

        default:
            break;
    }

    output.clear();
    output.m_prototype = prototype;  // now it's structured
    SQLBlock block;

    switch (queryBlock(cerberus::core::CerberusUtils::strPrint("SELECT * FROM %s;", tableName.c_str()), block).res)
    {
        case OR_QueryFailure:
            return OR_QueryFailure;
            break;

        case OR_DBFailure:
            return OR_DBFailure;
            break;

        case OR_NotFound:
            return OR_NotFound;
            break;

        default:
            break;
    }

    output.m_rows = block.m_rows;
    return OR_OK;
}
//=============================================================================
