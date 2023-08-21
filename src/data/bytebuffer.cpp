#include "bytebuffer.h"

#include <cstring>

#include "src/exception/exceptioncatalog.h"

using namespace cerberus::data;

//=============================================================================
void ByteBuffer::_resize(SIZE size)
{
    if (size == 0)
    {
        _clear();
        return;
    }

    BYTE* newbuf = (BYTE*)realloc(m_bytes, size);

    if (newbuf == nullptr)
    {
        throw cerberusSystemExc("could not reallocate ByteBuffer memory");
    }

    m_bytes = newbuf;
    m_size  = size;
    m_pos   = 0;
}
//=============================================================================
void ByteBuffer::_clear()
{
    free(m_bytes);
    m_bytes = nullptr;
    m_size  = 0;
    m_pos   = 0;
}
//=============================================================================
ByteBuffer::ByteBuffer(BYTE* buf, SIZE size)
    : m_bytes(buf),
      m_size(size),
      m_pos(0)
{
}
//=============================================================================
ByteBuffer::ByteBuffer(SIZE size)
    : m_bytes(nullptr),
      m_size(size),
      m_pos(0)
{
    if (size != 0)
    {
        m_bytes = (BYTE*)malloc(size);
    }
}
//=============================================================================
ByteBuffer::ByteBuffer(SIZE size, uint8_t val)
    : m_bytes(nullptr),
      m_size(size),
      m_pos(0)
{
    if (size != 0)
    {
        m_bytes = (BYTE*)malloc(size);
        if (m_bytes)
        {
            memset(m_bytes, val, size);
        }
    }
}
//=============================================================================
ByteBuffer::ByteBuffer()
    : m_bytes(nullptr),
      m_size(0),
      m_pos(0)
{
}
//=============================================================================
ByteBuffer::ByteBuffer(const ByteBuffer& other)
    : m_bytes(nullptr),
      m_size(0),
      m_pos(0)
{
    assign(other);
}
//=============================================================================
ByteBuffer::ByteBuffer(ByteBuffer&& other)
    : m_bytes(other.m_bytes),
      m_size(other.m_size),
      m_pos(other.m_pos)
{
    other.m_bytes = nullptr;
    other.m_size  = 0;
}
//=============================================================================
ByteBuffer::ByteBuffer(const char* str)
    : m_bytes(nullptr),
      m_size(0),
      m_pos(0)
{
    const char* c = str;

    while (*c)
    {
        c++;
    }

    SIZE s = c - str;

    if (s != 0)
    {
        m_bytes = (BYTE*)malloc(s);
        if (m_bytes)
        {
            memmove(m_bytes, str, s);
            m_size = s;
        }
    }
}
//=============================================================================
ByteBuffer::~ByteBuffer()
{
    if (m_bytes) free(m_bytes);
}
//=============================================================================
cerberus::BYTE* ByteBuffer::data() { return m_bytes; }
//=============================================================================
const cerberus::BYTE* ByteBuffer::data() const { return m_bytes; }
//=============================================================================
cerberus::BYTE ByteBuffer::at(SIZE index) const
{
    if (index >= m_size)
    {
        throw cerberusIllegalArgExc("index out of bound, %u/%u", index, m_size);
    }

    return *((unsigned char*)(m_bytes + index));
}
//=============================================================================
cerberus::BYTE ByteBuffer::operator[](SIZE index) const { return at(index); }
//=============================================================================
void ByteBuffer::appendFrom(const BYTE* buffer, SIZE len)
{
    ByteBuffer buf(len);
    BYTE* p = buf.data();
    memmove(p, buffer, len);

    append(buf);
}
//=============================================================================
void ByteBuffer::assignFrom(const BYTE* buffer, SIZE len)
{
    clear();
    appendFrom(buffer, len);
}
//=============================================================================
void ByteBuffer::copyTo(BYTE* buffer, SIZE maxLen) const
{
    if (m_size == 0 || !m_bytes)
    {
        return;
    }

    if (maxLen)
    {
        memmove(buffer, m_bytes, maxLen);
    }
    else
    {
        memmove(buffer, m_bytes, m_size);
    }
}
//=============================================================================
bool ByteBuffer::operator==(const ByteBuffer& other) const { return isEqual(other); }
//=============================================================================
bool ByteBuffer::operator!=(const ByteBuffer& other) const { return !isEqual(other); }
//=============================================================================
void ByteBuffer::operator+=(const ByteBuffer& other) { append(other); }
//=============================================================================
void ByteBuffer::operator+=(char c) { appendChar(c); }
//=============================================================================
ByteBuffer& ByteBuffer::operator=(const ByteBuffer& other)
{
    assign(other);
    return *this;
}
//=============================================================================
ByteBuffer& ByteBuffer::operator=(const char* str)
{
    assign(str);
    return *this;
}
//=============================================================================
ByteBuffer ByteBuffer::subBuffer(SIZE pos, SIZE len) const
{
    if (pos >= m_size)
    {
        return ByteBuffer();
    }

    if (pos + len >= m_size)
    {
        len = m_size - pos;
    }

    ByteBuffer ret(len);
    memmove(ret.data(), m_bytes + pos, ret.size());

    return ret;
}
//=============================================================================
ByteBuffer ByteBuffer::subBuffer(SIZE pos) const
{
    if (pos >= m_size)
    {
        return ByteBuffer();
    }

    ByteBuffer ret(m_size - pos);
    memmove(ret.data(), m_bytes + pos, ret.size());

    return ret;
}
//=============================================================================
ByteBuffer ByteBuffer::subBuffer_seek(SIZE len) const
{
    if (m_pos + len >= m_size)
    {
        auto p = m_pos;
        m_pos  = m_size;
        return subBuffer(p);
    }

    auto p = m_pos;
    m_pos += len;
    return subBuffer(p, len);
}
//=============================================================================
void ByteBuffer::appendString(const char* str)
{
    if (*str == 0)
    {
        return;
    }

    const char* c = str;

    while (*c)
    {
        c++;
    }

    SIZE s = c - str;

    SIZE oldSize = m_size;

    _resize(s + m_size);

    if (m_bytes)
    {
        memmove(m_bytes + oldSize, str, s);
    }
}
//=============================================================================
void ByteBuffer::appendChar(char c)
{
    SIZE s = m_size;

    s++;

    _resize(s);

    if (m_bytes)
    {
        *(m_bytes + s - 1) = c;
    }
}
//=============================================================================
cerberus::SIZE ByteBuffer::size() const { return m_size; }
//=============================================================================
void ByteBuffer::resize(SIZE size)
{
    if (size == 0)
    {
        clear();
        return;
    }

    _resize(size);
}
//=============================================================================
void ByteBuffer::assign(const ByteBuffer& other, SIZE len)
{
    if (m_bytes == other.m_bytes)  // same instance
    {
        return;
    }

    if (other.m_size == 0)
    {
        clear();
        return;
    }

    SIZE s;

    if (len)
        s = len;
    else
        s = other.m_size;

    _resize(s);

    memmove(m_bytes, other.m_bytes, s);
}
//=============================================================================
void ByteBuffer::assign(const char* str)
{
    _clear();

    const char* c = str;

    while (*c)
    {
        c++;
    }

    SIZE s = c - str;

    if (s != 0)
    {
        m_bytes = (uint8_t*)malloc(s);
        if (m_bytes)
        {
            memmove(m_bytes, str, s);
            m_size = s;
        }
    }
}
//=============================================================================
void ByteBuffer::append(const ByteBuffer& other)
{
    SIZE s = other.m_size;

    if (s == 0)
    {
        return;
    }

    SIZE oldSize = m_size;
    SIZE newSize = oldSize + s;

    _resize(newSize);

    if (m_bytes)
    {
        memmove(m_bytes + oldSize, other.m_bytes, s);
    }
}
//=============================================================================
void ByteBuffer::clear() { _clear(); }
//=============================================================================
bool ByteBuffer::isValid() const { return (m_size != 0 && m_bytes); }
//=============================================================================
bool ByteBuffer::isEqual(const ByteBuffer& other) const
{
    if (m_bytes == other.m_bytes)  // same instances
    {
        return true;
    }

    if (m_bytes == nullptr || other.m_bytes == nullptr || (m_size != other.m_size))
    {
        return false;
    }

    return (memcmp(m_bytes, other.m_bytes, m_size) == 0);
}
//=============================================================================
std::string ByteBuffer::toString() const
{
    std::string ret(m_size + 1, 0);
    copyTo((BYTE*)ret.data(), m_size);
    return ret;
}
//=============================================================================
std::string ByteBuffer::toNormalizedString() const
{
    auto str = toString();
    core::CerberusUtils::normalize(str);
    return str;
}
//=============================================================================
cerberus::OperationResult ByteBuffer::search(const char* str) const
{
    const char* c = str;

    while (*c)
    {
        c++;
    }

    SIZE s = c - str;

    if (s == 0 || s > m_size)
    {
        return OR_WrongArgument;
    }

    for (SIZE i = 0; i < m_size - s; i++)
    {
        for (SIZE j = 0; j < s; j++)
        {
            if (*(m_bytes + i + j) == *(str + j))
            {
                if (j == s - 1) return SIZE(i);
            }
            else
                break;
        }
    }

    return OR_NotFound;
}
//=============================================================================
std::string ByteBuffer::getLine() const
{
    std::string ret;

    char c    = 0;
    char prev = 0;

    while (true)
    {
        if (m_pos == m_size)  // end
        {
            return ret;
        }

        c = *(m_bytes + m_pos);

        if (c == '\n')
        {
            if (prev == '\r')
            {
                ret.pop_back();  // remove the \r
            }

            m_pos++;
            return ret;
        }

        ret.push_back(c);
        m_pos++;
        prev = c;
    }
}
//=============================================================================
void ByteBuffer::seek(SIZE pos) const
{
    if (pos >= m_size)
        m_pos = m_size;
    else
        m_pos = pos;
}
//=============================================================================
cerberus::SIZE ByteBuffer::pos() const { return m_pos; }
//=============================================================================
void ByteBuffer::resetCursor(bool end) const
{
    if (end)
        m_pos = m_size;
    else
        m_pos = 0;
}
//=============================================================================
bool ByteBuffer::end() const { return m_pos == m_size; }
//=============================================================================
