#ifndef CERBERUS_DATA_SHAREDBYTEBUFFER_H
#define CERBERUS_DATA_SHAREDBYTEBUFFER_H

#include "../Cerberus_global.h"
#include "../types.h"

/* This is the SharedByteBuffer class, a smart way to share data between threads
 *
 *
 * The SharedByteBuffer is a contiguos buffer with support for multi-threading.
 * It allocates the requested size buffer with no reserved space
 * (exactly the request amount will be allocated in the heap).
 *
 * The implementation follows the copy-on-write concept:
 *      When a SharedByteBuffer is copy-constructed (e.g. passed as parameter of a function)
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
        class ByteBuffer;

        class CERBERUS_EXPORT SharedByteBuffer
        {
           private:
            mutable ByteBuffer* m_buffer;   // heap
            mutable uint32_t* m_instances;  // heap
            mutable mutex::Mutex* m_mutex;  // heap
            mutable bool* m_hasOwner;       // heap
            mutable bool m_owner;

            void becomeOwner(bool force = false) const;

           public:
            // Construct a SharedByteBuffer allocating size bytes. The buffer is not initialized and may contain garbage
            SharedByteBuffer(SIZE size);

            // Construct a SharedByteBuffer allocating size bytes. The buffer is initialized using val value
            SharedByteBuffer(SIZE size, uint8_t val);

            // Construct a SharedByteBuffer using a given ByteBuffer instance.
            // Do not attempt to call any method on the ByteBuffer object after this call,
            // use the implementation of the SharedByteBuffer instead
            explicit SharedByteBuffer(ByteBuffer& buffer);

            // Construct an invalid SharedByteBuffer, no buffer memory allocation performed
            SharedByteBuffer();

            // Construct a SharedByteBuffer from another one. This operation will shallow-copy the buffer
            SharedByteBuffer(const SharedByteBuffer& other);

            // Move-construct a SharedByteBuffer, invalidating other
            SharedByteBuffer(SharedByteBuffer&& other);

            // Construct a SharedByteBuffer from a c-string. The string must end with a \0, that will NOT be copied
            SharedByteBuffer(const char* str);

            // Destroy the SharedByteBuffer instance and deallocate the memory ONLY if not used by another ByteBuffer
            virtual ~SharedByteBuffer();

            // Obtain the main pointer of the buffer. [OWN]
            BYTE* data();

            // Obtain the main pointer of the buffer (const version).
            const BYTE* data() const;

            // Append len bytes read from the given buffer to this SharedByteBuffer. [OWN]
            void appendFrom(const BYTE* buffer, SIZE len);

            // Assigns the content of the given buffer to this SharedByteBuffer. Existing content will be discarded. [OWN]
            void assignFrom(const BYTE* buffer, SIZE len);

            // Copy the content of this SharedByteBuffer to the given buffer. No more than maxLen bytes will be copied.
            // If maxLen is 0, all the content of this SharedByteBuffer will be copied
            void copyTo(BYTE* buffer, SIZE maxLen = 0);

            // Checks if this SharedByteBuffer instance is equal to other
            bool operator==(const SharedByteBuffer& other);

            // Checks if this SharedByteBuffer instance is different from other
            bool operator!=(const SharedByteBuffer& other);

            // Append the given SharedByteBuffer to the buffer. [OWN]
            void operator+=(const SharedByteBuffer& other);

            // Append the given char to the buffer. [OWN]
            void operator+=(unsigned char c);

            // Assign another SharedByteBuffer value to this instance, the memory is deep-copied. [OWN]
            SharedByteBuffer& operator=(const SharedByteBuffer& other);

            // Assign the c-string str value to this buffer.
            // After this call the size will be equal to the size of the given string. [OWN]
            SharedByteBuffer& operator=(const char* str);

            // Obtain another SharedByteBuffer instance that contains len bytes copied from position pos
            SharedByteBuffer subBuffer(SIZE pos, SIZE len);

            // Append the given c-string str to the end of this buffer.
            // The string must end with a \0, that will NOT be appended. [OWN]
            void appendString(const char* str);

            // Get the current buffer size
            SIZE size() const;

            // Change the size of the SharedByteBuffer.
            // If the new size is less than the current one, data loss may take place. [OWN]
            void resize(SIZE size);

            // Assign first len bytes of another SharedByteBuffer value to this instance, the memory is deep-copied. [OWN]
            // If len is 0, all the buffer is assigned
            void assign(const SharedByteBuffer& other, SIZE len = 0);

            // Assign the c-string str value to this buffer.
            // After this call the size will be equal to the size of the given string. [OWN]
            void assign(const char* str);

            // Append other buffer to this buffer. [OWN]
            void append(const SharedByteBuffer& other);

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

#endif  // CERBERUS_DATA_SHAREDBYTEBUFFER_H
