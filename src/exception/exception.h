#ifndef CERBERUS_EXCEPTION_EXCEPTION_H
#define CERBERUS_EXCEPTION_EXCEPTION_H

#include <cstdint>
#include <exception>
#include <string>

#include "../Cerberus_global.h"
#include "exceptioncatalog.h"

namespace cerberus
{
    namespace exception
    {
        class CERBERUS_EXPORT Exception : public std::exception
        {
           private:
            std::string m_error;

           public:
            enum ExceptionType
            {
                ET_Exception,              // generic exception
                ET_IllegalArgument,        // exception thrown when an argument is not valid
                ET_IllegalState,           // exception thrown when the state of an object is not valid
                ET_System,                 // exception thrown when system error occurs
                ET_MissingImplementation,  // exception thrown when a requested piece of code has not been implemented yet
                ET_InvalidCast,            // exception thrown for invalid cast operations
                ET_UsageError,             // exception thrown for a bad usage of the framework
                ET_OperationResult,        // exception type used by OpRes class
            };

            Exception() noexcept;

            Exception(const Exception& other) noexcept;

            Exception(const char* text, uint32_t line = 0, const char* fileName = nullptr, ExceptionType type = ET_Exception) noexcept;

            Exception& operator=(const Exception& other) noexcept;

            virtual ~Exception();

            virtual const char* what() const noexcept;
        };
    }  // namespace exception
}  // namespace cerberus
#endif  // CERBERUS_EXCEPTION_EXCEPTION_H
