#include "exception.h"
#include "../cerberus.h"

using namespace cerberus::exception;

//=============================================================================
Exception::Exception() noexcept
{
    m_error = "Unspecified exception";
}
//=============================================================================
Exception::Exception(const Exception& other) noexcept : m_error(other.m_error)
{
    // noop
}
//=============================================================================
Exception::Exception(const char* text, uint32_t line, const char* fileName, const char* type) noexcept
{
    std::string file(fileName);
    size_t slashPos = file.find_last_of("/\\");

    if(slashPos == std::string::npos)
    {
        slashPos = 0;
    }

    file = file.substr(slashPos);

    if(type == nullptr)
    {
        m_error = cerberus::Cerberus::strPrint("Unspecified exception in %s line %u: %s", fileName, line, text);
    }
    else
    {
        m_error = cerberus::Cerberus::strPrint("%s exception in %s line %u: %s", type, fileName, line, text);
    }
}
//=============================================================================
Exception& Exception::operator=(const Exception& other) noexcept
{
    m_error = other.m_error;
    return *this;
}
//=============================================================================
Exception::~Exception()
{
    // noop
}
//=============================================================================
const char* Exception::what() const noexcept
{
    return m_error.c_str();
}
//=============================================================================
