#ifndef CERBERUS_EXCEPTION_EXCEPTION_H
#define CERBERUS_EXCEPTION_EXCEPTION_H

#include <exception>
#include <string>
#include "../Cerberus_global.h"

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
                    ET_Unknown,
                    ET_IllegalArgument,
                    ET_IllegalState,
                    ET_System,
                    ET_MissingImplementation,
                };

                Exception() noexcept;

                Exception(const Exception& other) noexcept;

                Exception(const char* text, uint32_t line, const char* fileName, ExceptionType type = ET_Unknown) noexcept;

                Exception& operator= (const Exception& other) noexcept;

                virtual ~Exception();

                virtual const char* what() const noexcept;
        };
    }
}
#endif // CERBERUS_EXCEPTION_EXCEPTION_H
