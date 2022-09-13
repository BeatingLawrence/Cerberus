#include "sqlresult.h"
#include <pqxx/result>
#include <pqxx/row>

using namespace cerberus::data::database;

//=============================================================================
SQLResult::SQLResult() : m_result(new pqxx::result) {}
//=============================================================================
SQLResult::SQLResult(const SQLResult& other) : m_result(new pqxx::result(*other.m_result)) {}
//=============================================================================
SQLResult::~SQLResult()
{
    delete m_result;
}
//=============================================================================
SQLResult& SQLResult::operator=(const SQLResult& other)
{
    if(other.m_result != m_result)
    {
        delete m_result;
        m_result = new pqxx::result(*other.m_result);
    }

    return *this;
}
//=============================================================================
bool SQLResult::operator==(const SQLResult& other) const
{
    return *m_result == *other.m_result;
}
//=============================================================================
bool SQLResult::operator!=(const SQLResult& other) const
{
    return *m_result != *other.m_result;
}
//=============================================================================
SQLRow SQLResult::operator[](size_t pos) const
{
    SQLRow ret;
    delete ret.m_row;
    ret.m_row = new pqxx::row(m_result->at(pos));
    return ret;
}
//=============================================================================
size_t SQLResult::size() const
{
    return m_result->size();
}
//=============================================================================
bool SQLResult::isFailed() const
{
    return m_failed;
}
//=============================================================================
std::string SQLResult::failureReason() const
{
    return m_failureReason;
}
//=============================================================================
