#ifndef CERBERUS_DATA_BYTEBUFFER_H
#define CERBERUS_DATA_BYTEBUFFER_H

#include <cstdint>

#include "../Cerberus_global.h"
#include "src/types.h"

/* This is the ByteBuffer class, a smart way to share data between threads
 *
 *
 * The ByteBuffer is a contiguos buffer with support for multi-threading.
 * It allocates the requested size buffer with no reserved space
 * (exactly the request amount will be allocated in the heap).
 *
 * The implementation follows the copy-on-write concept:
 *      When a ByteBuffer is copy-constructed (e.g. passed as parameter of a function)
 *      its buffer is not immediately deep-copied. Instead, just a reference to the heap
 *      allocated memory is shared, and an instance counter is incremented.
 *
 *      When the calling thread performs a read call, the shared buffer is being accessed.
 *
 *      When the calling thread performs a write call, the ownership is forced, so the
 *      memory is deep-copied first, and then modified.
 *
 *      This mechanism makes the read-only instances copies more
 *      efficient, expecially when the buffer is large.
 *
 *
 *      The calls marked with the [OWN] tag are those who forcefully deep-copy the buffer before use it.
 *      Please note that if the owner instance is destroyed, the buffer is marked as 'not owned', and
 *      after that, any instance that refers to that buffer will TRY to own the buffer at any call, even read ones.
 *
 *
 *      [OWN]: Please note that this method will force this instance to be the owner of the buffer,
 *             thus, the buffer will be deep-copied if necessary
 */

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

            // Obtain the main pointer of the buffer. [OWN]
            unsigned char* data();

            // Obtain the main pointer of the buffer (const version).
            const unsigned char* data() const;

            // Obtain a single element by value.
            // An exception will be thrown if index >= size()
            unsigned char operator[](SIZE index);

            // Append len bytes read from the given buffer to this ByteBuffer. [OWN]
            void appendFrom(const char* buffer, SIZE len);

            // Assigns the content of the given buffer to this ByteBuffer. Existing content will be discarded. [OWN]
            void assignFrom(const char* buffer, SIZE len);

            // Copy the content of this ByteBuffer to the given buffer. No more than maxLen bytes will be copied.
            // If maxLen is 0, all the content of this ByteBuffer will be copied
            void copyTo(char* buffer, SIZE maxLen = 0);

            // Checks if this ByteBuffer instance is equal to other
            bool operator==(const ByteBuffer& other);

            // Checks if this ByteBuffer instance is different from other
            bool operator!=(const ByteBuffer& other);

            // Append the given ByteBuffer to the buffer. [OWN]
            void operator+=(const ByteBuffer& other);

            // Append the given char to the buffer. [OWN]
            void operator+=(unsigned char c);

            // Assign another ByteBuffer value to this instance, the memory is deep-copied. [OWN]
            ByteBuffer& operator=(const ByteBuffer& other);

            // Assign the c-string str value to this buffer.
            // After this call the size will be equal to the size of the given string. [OWN]
            ByteBuffer& operator=(const char* str);

            // Obtain another ByteBuffer instance that contains len bytes copied from position pos
            ByteBuffer subBuffer(SIZE pos, SIZE len);

            // Append the given c-string str to the end of this buffer.
            // The string must end with a \0, that will NOT be appended. [OWN]
            void appendString(const char* str);

            // Get the current buffer size
            SIZE size() const;

            // Change the size of the ByteBuffer.
            // If the new size is less than the current one, data loss may take place. [OWN]
            void resize(SIZE size);

            // Assign another ByteBuffer value to this instance, the memory is deep-copied. [OWN]
            void assign(const ByteBuffer& buffer);

            // Assign the c-string str value to this buffer.
            // After this call the size will be equal to the size of the given string. [OWN]
            void assign(const char* str);

            // Append other buffer to this buffer. [OWN]
            void append(const ByteBuffer& other);

            // Clear this buffer completely, deallocating all the memory. [OWN]
            void clear();

            // Check if the instance is valid
            bool isValid();

            // Become the owner of the buffer, effectively deep-copying it if necessary
            void appropriate();

            // Get the number of instances which refer to this buffer
            uint32_t instances();
        };
    }  // namespace data
}  // namespace cerberus

#endif  // CERBERUS_DATA_BYTEBUFFER_H
