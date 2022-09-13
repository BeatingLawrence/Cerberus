#include "sqldatabase.h"
#include <pqxx/connection>
#include <pqxx/transaction>
#include "sqlresult.h"
#include "sqlrow.h"

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
    catch(std::exception e)
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
    delete result.m_result;

    if(m_failed)    //return a failed result if connection is in a failed state
    {
        result.m_failed = true;
        result.m_failureReason = "Connection error: " + m_failureReason;
        return result;
    }

    try
    {
        pqxx::work w(*m_connection);
        result.m_result = new pqxx::result(w.exec(query));
        w.commit();
    }
    catch(pqxx::sql_error e)
    {
        result.m_failed = true; //return a failed result if query failed
        result.m_failureReason = e.what();
    }
    catch(std::exception e)
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
    delete row.m_row;

    if(m_failed)    //return a failed result if connection is in a failed state
    {
        row.m_failed = true;
        row.m_failureReason = "Connection error: " + m_failureReason;
        return row;
    }

    try
    {
        pqxx::work w(*m_connection);
        row.m_row = new pqxx::row(w.exec1(query));
        w.commit();
    }
    catch(pqxx::sql_error e)
    {
        row.m_failed = true; //return a failed result if query failed
        row.m_failureReason = e.what();
    }
    catch(std::exception e)
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
        return false;
    }
    catch(std::exception e)
    {
        m_failed = true;
        m_failureReason = e.what();
        return false;
    }

    return true;
}
//=============================================================================
