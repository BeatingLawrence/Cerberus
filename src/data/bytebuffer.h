#ifndef CERBERUS_DATA_BYTEBUFFER_H
#define CERBERUS_DATA_BYTEBUFFER_H

#include <cstdint>

#include "../Cerberus_global.h"
#include "src/types.h"

namespace cerberus
{
    namespace mutex
    {
        class Mutex;
    }
    namespace data
    {
        class CERBERUS_EXPORT ByteBuffer
        {
           private:
            mutable uint8_t* m_bytes;       // heap
            mutable uint32_t* m_instances;  // heap
            mutable mutex::Mutex* m_mutex;  // heap
            mutable SIZE* m_size;           // heap
            mutable bool* m_hasOwner;       // heap
            mutable bool m_owner;

            void becomeOwner(bool force = false) const;

            void _resize(SIZE size);

            void _clear();

           public:
            // Construct a ByteBuffer allocating size bytes. The buffer is not initialized and may contain garbage
            ByteBuffer(SIZE size);

            // Construct a ByteBuffer allocating size bytes. The buffer is initialized using val value
            ByteBuffer(SIZE size, uint8_t val);

            // Construct an invalid ByteBuffer, no buffer memory allocation performed
            ByteBuffer();

            // Construct a ByteBuffer from another one. This operation will shallow-copy the buffer
            ByteBuffer(const ByteBuffer& other);

            // Move-construct a ByteBuffer, invalidating other
            ByteBuffer(ByteBuffer&& other);

            // Construct a ByteBuffer from a c-string. The string must end with a \0, that will NOT be copied
            ByteBuffer(const char* str);

            // Destroy the ByteBuffer instance and deallocate the memory ONLY if not used by another ByteBuffer
            virtual ~ByteBuffer();

            // Obtain the main pointer of the buffer.
            // Please note that this method will force this instance to be the owner of the buffer,
            // thus, the buffer will be deep-copied if necessary
            unsigned char* data();

            // Obtain the main pointer of the buffer (const version).
            const unsigned char* data() const;

            // Obtain a single element by reference.
            // An exception will be thrown if index >= size()
            // Please note that this method will force this instance to be the owner of the buffer,
            // thus, the buffer will be deep-copied if necessary
            unsigned char& operator[](SIZE index);

            // Checks if this ByteBuffer instance is equal to other
            bool operator==(const ByteBuffer& other);

            // Checks if this ByteBuffer instance is different from other
            bool operator!=(const ByteBuffer& other);

            // Append the given ByteBuffer to the buffer
            // Please note that this method will force this instance to be the owner of the buffer,
            // thus, the buffer will be deep-copied if necessary
            void operator+=(const ByteBuffer& other);

            // Append the given char to the buffer
            // Please note that this method will force this instance to be the owner of the buffer,
            // thus, the buffer will be deep-copied if necessary
            void operator+=(unsigned char c);

            // Assign another ByteBuffer value to this instance, the memory is deep-copied
            // Please note that this method will force this instance to be the owner of the buffer,
            // thus, the buffer will be deep-copied if necessary
            ByteBuffer& operator=(const ByteBuffer& other);

            // Assign the c-string str value to this buffer.
            // After this call the size will be equal to the size of the given string
            // Please note that this method will force this instance to be the owner of the buffer,
            // thus, the buffer will be deep-copied if necessary
            ByteBuffer& operator=(const char* str);

            // Obtain another ByteBuffer instance that contains len bytes copied from position pos
            ByteBuffer subBuffer(SIZE pos, SIZE len);

            // Append the given c-string str to the end of this buffer.
            // The string must end with a \0, that will NOT be appended
            // Please note that this method will force this instance to be the owner of the buffer,
            // thus, the buffer will be deep-copied if necessary
            void appendString(const char* str);

            // Get the current buffer size
            SIZE size() const;

            // Change the size of the ByteBuffer.
            // If the new size is less than the current one, data loss may take place
            // Please note that this method will force this instance to be the owner of the buffer,
            // thus, the buffer will be deep-copied if necessary
            void resize(SIZE size);

            // Assign another ByteBuffer value to this instance, the memory is deep-copied
            // Please note that this method will force this instance to be the owner of the buffer,
            // thus, the buffer will be deep-copied if necessary
            void assign(const ByteBuffer& buffer);

            // Assign the c-string str value to this buffer.
            // After this call the size will be equal to the size of the given string
            // Please note that this method will force this instance to be the owner of the buffer,
            // thus, the buffer will be deep-copied if necessary
            void assign(const char* str);

            // Append other buffer to this buffer
            // Please note that this method will force this instance to be the owner of the buffer,
            // thus, the buffer will be deep-copied if necessary
            void append(const ByteBuffer& other);

            // Clear this buffer completely, deallocating all the memory
            // Please note that this method will force this instance to be the owner of the buffer,
            // thus, the buffer will be deep-copied if necessary
            void clear();

            // Check if the instance is valid
            bool isValid();

            // Become the owner of the buffer, effectively deep-copying it if necessary
            void appropriate();
        };
    }  // namespace data
}  // namespace cerberus

#endif  // CERBERUS_DATA_BYTEBUFFER_H
