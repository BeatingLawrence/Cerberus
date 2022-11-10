#include "sqldatabase.h"
#include <pqxx/connection>
#include <pqxx/result_iterator.hxx>
#include <pqxx/transaction>
#include "sqlresult.h"
#include "sqlrow.h"
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
SQLResult SQLDatabase::query(const std::string& query)
{
    SQLResult result;

    if(m_failed)    //return a failed result if connection is in a failed state
    {
        result.m_failed = true;
        result.m_failureReason = "Connection error: " + m_failureReason;
        return result;
    }

    try
    {
        pqxx::work w(*m_connection);
        pqxx::result res = w.exec(query);

        for(auto&& row : res)
        {
            SQLRow r;

            for(auto&& val : row)
            {
                r.append(val.c_str());
            }

            result.append(r);
        }

        w.commit();
    }
    catch(pqxx::sql_error e)
    {
        logError(e.what());
        logError(e.sqlstate());
        result.m_failed = true; //return a failed result if query failed
        result.m_failureReason = e.what();
    }
    catch(pqxx::failure e)
    {
        result.m_failed = true; //return a failed result if connection failed
        result.m_failureReason = "Connection error: " + std::string(e.what());
        m_failed = true;
        m_failureReason = e.what();
    }

    return result;
}
//=============================================================================
SQLRow SQLDatabase::querySingleRow(const std::string& query)
{
    SQLRow row;

    if(m_failed)    //return a failed row if connection is in a failed state
    {
        row.m_failed = true;
        row.m_failureReason = "Connection error: " + m_failureReason;
        return row;
    }

    try
    {
        pqxx::work w(*m_connection);
        pqxx::row r = w.exec1(query);

        for(auto&& val : r)
        {
            row.append(val.c_str());
        }

        w.commit();
    }
    catch(pqxx::sql_error e)
    {
        logError(e.what());
        logError(e.sqlstate());
        row.m_failed = true; //return a failed result if query failed
        row.m_failureReason = e.what();
    }
    catch(pqxx::failure e)
    {
        row.m_failed = true; //return a failed result if connection failed
        row.m_failureReason = "Connection error: " + std::string(e.what());
        m_failed = true;
        m_failureReason = e.what();
    }

    return row;
}
//=============================================================================
bool SQLDatabase::command(const std::string& query)
{
    if(m_failed)
    {
        return false;
    }

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
        return false;
    }
    catch(pqxx::failure e)
    {
        m_failed = true;
        m_failureReason = e.what();
        return false;
    }

    return true;
}
//=============================================================================
