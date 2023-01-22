#include "sqlblock.h"
#include "./sqlrow.h"
#include <pqxx/result>
#include "../../core/cerberuslog.h"

using namespace cerberus::data::database;

//=============================================================================
SQLBlock::SQLBlock(const SQLRow& row)
{
    append(row);
}
//=============================================================================
SQLBlock::~SQLBlock() {}
//=============================================================================
bool SQLBlock::isFailed() const
{
    return m_failed;
}
//=============================================================================
std::string SQLBlock::failureReason() const
{
    return m_failureReason;
}
//=============================================================================
SQLBlock& SQLBlock::operator=(const SQLBlock& other)
{
    m_rows = other.m_rows;
    return *this;
}
//=============================================================================
void SQLBlock::append(const SQLRow& row)
{
    if(m_rows.empty())
    {
        m_columns = row.size();
    }

    if(row.size() != m_columns)
    {
        logError("Refusing to append a row to a block of different size");
        return;
    }

    m_rows.push_back(row);
}
//=============================================================================
size_t SQLBlock::size() const
{
    return m_rows.size();
}
//=============================================================================
bool SQLBlock::empty() const
{
    return size() == 0;
}
//=============================================================================
void SQLBlock::clear()
{
    m_rows.clear();
    m_columns = 0;
    m_failureReason = "";
    m_failed = false;
}
//=============================================================================
SQLRow SQLBlock::operator[](size_t pos) const
{
    return m_rows[pos];
}
//=============================================================================
bool SQLBlock::operator==(const SQLBlock& other) const
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
bool SQLBlock::operator!=(const SQLBlock& other) const
{
    return !((*this) == other);
}
//=============================================================================
