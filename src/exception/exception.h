#ifndef CERBERUS_EXCEPTION_EXCEPTION_H
#define CERBERUS_EXCEPTION_EXCEPTION_H

#include <exception>
#include <string>

namespace cerberus
{
    namespace exception
    {
        class Exception : public std::exception
        {
            private:
                std::string m_error;

            public:
                Exception() noexcept;

                Exception(const Exception& other) noexcept;

                Exception(const char* text, uint32_t line, const char* fileName, const char* type = nullptr) noexcept;

                Exception& operator= (const Exception& other) noexcept;

                virtual ~Exception();

                virtual const char* what() const noexcept;
        };
    }
}
#endif // CERBERUS_EXCEPTION_EXCEPTION_H
