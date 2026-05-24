#include "exception.h"

#include <cstdarg>

#include "../core/cerberusutils.h"

using namespace crb;

//=============================================================================
Exception::Exception() noexcept { m_error = "Unspecified exception"; }
//=============================================================================
Exception::Exception(const Exception& other) noexcept
    : m_error(other.m_error)
{
    // noop
}
//=============================================================================
Exception::Exception(ExceptionType type, uint32_t line, const char* fileName, const char* format,
                     ...) noexcept
{
    switch (type)
    {
        case ET_IllegalArgument:
            m_error = "Illegal argument";
            break;

        case ET_IllegalState:
            m_error = "Illegal state";
            break;

        case ET_System:
            m_error = "System error";
            break;

        case ET_MissingImplementation:
            m_error = "Missing implementation";
            break;

        case ET_InvalidCast:
            m_error = "Invalid cast";
            break;

        case ET_UsageError:
            m_error = "Usage error";
            break;

        case ET_OperationResult:
            m_error = "Operation error";
            break;

        case ET_Exception:
            m_error = "Generic error";
            break;

        case ET_Fatal:
            m_error = "Fatal error";
            break;
    }

    if (fileName)
    {
        std::string file(fileName);
        if (!file.empty())
        {
            size_t slashPos = file.find_last_of("/\\");

            if (slashPos == std::string::npos)
            {
                slashPos = 0;
            }

            file = file.substr(++slashPos);
            m_error += " in ";
            m_error += file;
        }
    }

    if (line) m_error += CerberusUtils::strPrint(":%u", line);
    std::string text;

    // creating string
    if (format)
    {
        va_list list;
        va_start(list, format);
        text = CerberusUtils::strPrint_valist(format, list);
        va_end(list);
    }
    //

    if (!text.empty())
    {
        m_error.append(", ");
        m_error.append(text);
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
const char* Exception::what() const noexcept { return m_error.c_str(); }
//=============================================================================
