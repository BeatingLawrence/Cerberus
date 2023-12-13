#ifndef CERBERUS_DATA_BYTEBUFFER_H
#define CERBERUS_DATA_BYTEBUFFER_H

#include "../Cerberus_global.h"
#include "../types.h"

namespace cerberus
{
    namespace data
    {
        class SharedByteBuffer;

        class CERBERUS_EXPORT ByteBuffer
        {
            friend class cerberus::data::SharedByteBuffer;

           private:
            BYTE* m_bytes;  // heap
            SIZE m_size;
            mutable SIZE m_pos;

            void _resize(SIZE size);

            void _clear();

            BYTE& getat(SIZE index) const;

            ByteBuffer(BYTE* buf, SIZE size);

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
            BYTE* data();

            // Obtain the main pointer of the buffer (const version).
            const BYTE* data() const;

            // Obtain a single element by reference.
            // An exception will be thrown if index >= size()
            const BYTE& at(SIZE index) const;
            BYTE& at(SIZE index);

            // Obtain a single element by value.
            // An exception will be thrown if index >= size()
            BYTE& operator[](SIZE index);

            // Append len bytes read from the given buffer to this ByteBuffer.
            void appendFrom(const BYTE* buffer, SIZE len);

            // Assigns the content of the given buffer to this ByteBuffer. Existing content will be discarded.
            void assignFrom(const BYTE* buffer, SIZE len);

            // Copy the content of this ByteBuffer to the given buffer. No more than maxLen bytes will be copied.
            // If maxLen is 0, all the content of this ByteBuffer will be copied
            void copyTo(BYTE* buffer, SIZE maxLen = 0) const;

            // Checks if this ByteBuffer instance is equal to other
            bool operator==(const ByteBuffer& other) const;

            // Checks if this ByteBuffer instance is different from other
            bool operator!=(const ByteBuffer& other) const;

            // Append the given ByteBuffer to the buffer.
            void operator+=(const ByteBuffer& other);

            // Append the given char to the buffer.
            void operator+=(char c);

            // Assign another ByteBuffer value to this instance, the memory is deep-copied.
            ByteBuffer& operator=(const ByteBuffer& other);

            // Assign the c-string str value to this buffer.
            // After this call the size will be equal to the size of the given string.
            ByteBuffer& operator=(const char* str);

            // Obtain another ByteBuffer instance that contains len bytes copied from position pos
            // If the pos is greater than or equal to the size, this call will return an empty buffer.
            // If specified len is too large, the operation will copy the buffer till the end.
            ByteBuffer subBuffer(SIZE pos, SIZE len) const;

            // Obtain another ByteBuffer instance that contains bytes copied from position pos till the end of the buffer
            // If the pos is greater than or equal to the size, this call will return an empty buffer.
            ByteBuffer subBuffer(SIZE pos) const;

            // Obtain another ByteBuffer instance that contains len bytes copied from current cursor position.
            // The cursor position is incremented by len.
            // If specified len is too large, the operation will copy the buffer till the end.
            // If the current cursor position is at the end, this call will return an empty buffer.
            ByteBuffer subBuffer_seek(SIZE len) const;

            // Append the given c-string str to the end of this buffer.
            // The string must end with a \0, that will NOT be appended.
            void appendString(const char* str);

            // Appens the given char to the end of this buffer.
            // The size will be incremented by one.
            void appendChar(char c);

            // Get the current buffer size
            SIZE size() const;

            // Change the size of the ByteBuffer.
            // If the new size is less than the current one, data loss may take place.
            void resize(SIZE size);

            // Assign first len bytes of another ByteBuffer value to this instance, the memory is deep-copied.
            // If len is 0, all the buffer is assigned
            void assign(const ByteBuffer& other, SIZE len = 0);

            // Assign the c-string str value to this buffer.
            // After this call the size will be equal to the size of the given string.
            void assign(const char* str);

            // Append other buffer to this buffer.
            void append(const ByteBuffer& other);

            // Clear this buffer completely, deallocating all the memory.
            void clear();

            // Check if the instance is valid
            bool isValid() const;

            // Return true if the content of this buffer matches against the content of other buffer
            bool isEqual(const ByteBuffer& other) const;

            // Return the buffer content as a string
            std::string toString() const;

            // Return the buffer content as a normalized string (it contains alpha-numeric,
            // symbols, and cr lf and crlf are substituted by "\r" "\n" and "\r\n")
            std::string toNormalizedString() const;

            // Return the buffer content as a HEX binary data string
            // The align argument specifies the number of bytes written per raw.
            // A value of zero disables the alignment
            std::string toBinaryDumpString(uint32_t align = 0) const;

            // Search a sequence of chars and return its position in the size field.
            // If the given string is empty or if its size is larger than the buffer size,
            // this method returns OR_WrongArgument.
            // If str is not contained inside the buffer, this method returns OR_NotFound
            OperationResult search(const char* str) const;

            // Return data until one of the following sequences is found:
            // { "\n", "\r\n"}
            // The sequence is not inserted in the returned string
            std::string getLine() const;

            // Seek to the position pos
            void seek(SIZE pos) const;

            // Return the current cursor position
            SIZE pos() const;

            // Seek to zero if end == false,
            // Seek to the last byte if end == true
            void resetCursor(bool end = false) const;

            // Tell if the cursor is at the end (no more data can be read)
            bool end() const;
        };
    }  // namespace data
}  // namespace cerberus

#endif  // CERBERUS_DATA_BYTEBUFFER_H
