#include "sqlresult.h"
#include "./sqlrow.h"
#include <pqxx/result>
#include "../../core/cerberuslog.h"

using namespace cerberus::data::database;

//=============================================================================
SQLResult::~SQLResult() {}
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
SQLResult& SQLResult::operator=(const SQLResult& other)
{
    m_rows = other.m_rows;
    return *this;
}
//=============================================================================
void SQLResult::append(const SQLRow& row)
{
    if(m_rows.empty())
    {
        m_columns = row.size();
    }

    if(row.size() != m_columns)
    {
        logError("Refusing to append a row to a result of different size");
        return;
    }

    m_rows.push_back(row);
}
//=============================================================================
size_t SQLResult::size() const
{
    return m_rows.size();
}
//=============================================================================
SQLRow SQLResult::operator[](size_t pos) const
{
    return m_rows[pos];
}
//=============================================================================
bool SQLResult::operator==(const SQLResult& other) const
{
    if(size() != other.size() || m_columns != other.m_columns)  //be sure rows and columns numbers are equal
    {
        return false;
    }

    for(size_t i = 0; i < size(); i++)
    {
        for(size_t j = 0; j < m_columns; j++)
        {
            if(m_rows[i][j].compare(other.m_rows[i][j]) != 0)
            {
                return false;
            }
        }
    }

    return true;
}
//=============================================================================
bool SQLResult::operator!=(const SQLResult& other) const
{
    return !((*this) == other);
}
//=============================================================================
