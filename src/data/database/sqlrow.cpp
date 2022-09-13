#include "sqlrow.h"
#include <pqxx/row>

using namespace cerberus::data::database;

//=============================================================================
SQLRow::SQLRow() : m_row(new pqxx::row) {}
//=============================================================================
SQLRow::SQLRow(const SQLRow& other) : m_row(new pqxx::row(*other.m_row)) {}
//=============================================================================
SQLRow::SQLRow(SQLRow&& other) : m_row(other.m_row)
{
    other.m_row = nullptr;
}
//=============================================================================
SQLRow::~SQLRow()
{
    delete m_row;
}
//=============================================================================
bool SQLRow::isFailed() const
{
    return m_failed;
}
//=============================================================================
std::string SQLRow::failureReason() const
{
    return m_failureReason;
}
//=============================================================================
SQLRow& SQLRow::operator=(const SQLRow& other)
{
    if(other.m_row != m_row)
    {
        delete m_row;
        m_row = new pqxx::row(*other.m_row);
    }

    return *this;
}
//=============================================================================
size_t SQLRow::size() const
{
    return m_row->size();
}
//=============================================================================
template<typename... TYPE>
std::tuple<TYPE...> SQLRow::to() const
{
    return m_row->as<TYPE...>();
}
//=============================================================================
