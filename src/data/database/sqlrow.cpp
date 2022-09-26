#include "sqlrow.h"
#include <stdarg.h>

using namespace cerberus::data::database;

//=============================================================================
SQLRow::~SQLRow()
{
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
    m_values = other.m_values;
    return *this;
}
//=============================================================================
void SQLRow::append(const std::string& value)
{
    m_values.push_back(value);
}
//=============================================================================
size_t SQLRow::size() const
{
    return m_values.size();
}
//=============================================================================
std::string SQLRow::operator [](size_t pos) const
{
    return m_values[pos];
}
//=============================================================================
