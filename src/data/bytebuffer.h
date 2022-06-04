#ifndef CERBERUS_DATA_BYTEBUFFER_H
#define CERBERUS_DATA_BYTEBUFFER_H

#include <vector>
#include "../Cerberus_global.h"

namespace cerberus
{
    namespace data
    {
        class CERBERUS_EXPORT ByteBuffer
        {
            private:
                std::vector<unsigned char>* m_bytes;

            public:
                ByteBuffer(std::size_t size);

                ByteBuffer();

                ByteBuffer(const ByteBuffer& other);

                ByteBuffer(ByteBuffer&& other);

                //Does not take the \0
                ByteBuffer(const char* chars);

                virtual ~ByteBuffer();

                unsigned char* data();

                const unsigned char* data() const;

                unsigned char& operator [](std::size_t index);

                const unsigned char& operator [](std::size_t index) const;

                ByteBuffer subBuffer(std::size_t pos, std::size_t len);

                bool operator ==(const ByteBuffer& other);

                void operator +=(const ByteBuffer& other);

                void operator +=(unsigned char c);

                void appendString(const char* chars);

                std::size_t size() const;

                void resize(std::size_t size);
        };
    }
}

#endif // CERBERUS_DATA_BYTEBUFFER_H
