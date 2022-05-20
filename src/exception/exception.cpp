#include "exception.h"
#include "../core/cerberusutils.h"

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
Exception::Exception(const char* text, uint32_t line, const char* fileName, ExceptionType type) noexcept
{
    std::string file(fileName);
    size_t slashPos = file.find_last_of("/\\");

    if(slashPos == std::string::npos)
    {
        slashPos = 0;
    }

    file = file.substr(++slashPos);

    switch(type)
    {
        case ET_Unknown:
            m_error = core::CerberusUtils::strPrint("Unknown exception in %s:%u, %s", file.c_str(), line, text);
            break;

        case ET_IllegalArgument:
            m_error = core::CerberusUtils::strPrint("Illegal argument exception in %s:%u, %s", file.c_str(), line, text);
            break;

        case ET_IllegalState:
            m_error = core::CerberusUtils::strPrint("Illegal state exception in %s:%u, %s", file.c_str(), line, text);
            break;

        case ET_System:
            m_error = core::CerberusUtils::strPrint("System exception in %s:%u, %s", file.c_str(), line, text);
            break;

        case ET_MissingImplementation:
            m_error = core::CerberusUtils::strPrint("Missing implementation exception in %s:%u, %s", file.c_str(), line, text);
            break;
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
