#include "sqldatabase.h"

#include "../../exception/exception.h"
#include "dbdata.h"

using namespace cerberus;

//=============================================================================
SQLDatabase::SQLDatabase(DBBackend backend)
    : Database(backend)
{
    if (backend == DBB_Filesystem) throw cIllegalArgExc("DBB_Filesystem is not an SQL backend");
}
//=============================================================================
SQLDatabase::~SQLDatabase() {}
//=============================================================================
cerberus::OpRes SQLDatabase::createTable(DBTableProto& prototype)
{
    auto name = prototype.name();

    if (name.empty()) return OR_WrongArgument;

    auto res = queryPrototype(name);

    if (res.ok()) return OR_TableAlreadyPresent;

    if (res.res != OR_NotFound) return res;

    std::string cmd("CREATE TABLE ");
    cmd += prototype.name();
    cmd += " ( ";

    for (auto&& el : prototype)
    {
        auto type = el.type();

        if (type == SQLDataType::SDT_Bit || type == SQLDataType::SDT_VarBit ||
            type == SQLDataType::SDT_Char ||
            type == SQLDataType::SDT_VarChar)  // check if the mod parameter should be printed
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
cerberus::OpRes SQLDatabase::insertBlock(const DBTableBlock& block)
{
    if (block.m_prototype.name().empty()) return OR_WrongArgument;

    std::string cmd("INSERT INTO ");
    cmd += block.m_prototype.name();
    cmd += " VALUES ";

    for (auto&& row : block.m_rows)
    {
        cmd += '(';

        for (int i = 0; i < row.size(); i++)
        {
            auto type = block.prototype()[i].type();

            if (type == SQLDataType::SDT_Char ||
                type == SQLDataType::SDT_VarChar)  // check if value is a number or a string
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
cerberus::OpRes SQLDatabase::dropTable(const std::string& table)
{
    return command(CerberusUtils::strPrint("DROP TABLE %s;", table.c_str()));
}
//=============================================================================
OpResData<DBTableBlock> SQLDatabase::querytable(const std::string& tableName)
{
    auto proto = queryPrototype(tableName);

    if (proto.fail()) return proto;

    DBTableBlock ret(proto.value);

    auto q = queryBlock(CerberusUtils::strPrint("SELECT * FROM %s;", tableName.c_str()));

    if (q.fail()) return q;

    DBTableBlock block = q.value;

    ret.m_rows = block.m_rows;

    return ret;
}
//=============================================================================
