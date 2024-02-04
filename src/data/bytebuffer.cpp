#include "bytebuffer.h"

#include <cstdlib>
#include <cstring>

#include "src/exception/exception.h"

using namespace cerberus::data;
using namespace cerberus;

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
    if (m_bytes) free(m_bytes);
    m_bytes = nullptr;
    m_size  = 0;
    m_pos   = 0;
}
//=============================================================================
cerberus::BYTE& ByteBuffer::getat(SIZE index) const { return *((unsigned char*)(m_bytes + index)); }
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

        if (m_bytes == nullptr)
        {
            throw cerberusSystemExc("could not allocate ByteBuffer memory");
        }
    }
}
//=============================================================================
ByteBuffer::ByteBuffer(SIZE size, BYTE val)
    : m_bytes(nullptr),
      m_size(size),
      m_pos(0)
{
    if (size != 0)
    {
        m_bytes = (BYTE*)malloc(size);

        if (m_bytes == nullptr)
        {
            throw cerberusSystemExc("could not allocate ByteBuffer memory");
        }
        else
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
ByteBuffer::ByteBuffer(const std::string& str)
    : m_bytes(nullptr),
      m_size(0),
      m_pos(0)
{
    if (str.empty()) return;

    SIZE s  = str.size();
    m_bytes = (BYTE*)malloc(s);
    if (m_bytes)
    {
        memmove(m_bytes, str.data(), s);
        m_size = s;
    }
}
//=============================================================================
ByteBuffer::~ByteBuffer() { _clear(); }
//=============================================================================
ConstIterator<BYTE> ByteBuffer::begin() const
{
    if (isEmpty()) return nullptr;
    return &getat(0);
}
//=============================================================================
ConstIterator<BYTE> ByteBuffer::end() const
{
    if (isEmpty()) return nullptr;
    return &getat(m_size);
}
//=============================================================================
Iterator<BYTE> ByteBuffer::begin()
{
    if (isEmpty()) return nullptr;
    return &getat(0);
}
//=============================================================================
Iterator<BYTE> ByteBuffer::end()
{
    if (isEmpty()) return nullptr;
    return &getat(m_size);
}
//=============================================================================
cerberus::BYTE* ByteBuffer::data() { return m_bytes; }
//=============================================================================
const cerberus::BYTE* ByteBuffer::data() const { return m_bytes; }
//=============================================================================
const cerberus::BYTE& ByteBuffer::at(SIZE index) const
{
    if (index >= m_size)
    {
        throw cerberusIllegalArgExc("index out of bound, %u/%u", index, m_size);
    }
    return getat(index);
}
//=============================================================================
cerberus::BYTE& ByteBuffer::at(SIZE index)
{
    if (index >= m_size)
    {
        throw cerberusIllegalArgExc("index out of bound, %u/%u", index, m_size);
    }
    return getat(index);
}
//=============================================================================
cerberus::BYTE& ByteBuffer::operator[](SIZE index) { return at(index); }
//=============================================================================
ByteBuffer& ByteBuffer::appendFrom(const BYTE* buffer, SIZE len)
{
    ByteBuffer buf(len);
    BYTE* p = buf.data();
    memmove(p, buffer, len);

    append(buf);

    return *this;
}
//=============================================================================
ByteBuffer& ByteBuffer::assignFrom(const BYTE* buffer, SIZE len)
{
    clear();
    appendFrom(buffer, len);

    return *this;
}
//=============================================================================
const ByteBuffer& ByteBuffer::copyTo(BYTE* buffer, SIZE maxLen) const
{
    if (m_size == 0 || !m_bytes) return *this;

    if (maxLen)
        memmove(buffer, m_bytes, maxLen);
    else
        memmove(buffer, m_bytes, m_size);

    return *this;
}
//=============================================================================
bool ByteBuffer::operator==(const ByteBuffer& other) const { return isEqual(other); }
//=============================================================================
bool ByteBuffer::operator!=(const ByteBuffer& other) const { return !isEqual(other); }
//=============================================================================
ByteBuffer& ByteBuffer::operator+=(const ByteBuffer& other)
{
    append(other);
    return *this;
}
//=============================================================================
ByteBuffer& ByteBuffer::operator+=(char c)
{
    appendChar(c);
    return *this;
}
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
    if (pos >= m_size) return ByteBuffer();

    if (pos + len >= m_size) len = m_size - pos;

    ByteBuffer ret(len);
    memmove(ret.data(), m_bytes + pos, len);

    return ret;
}
//=============================================================================
ByteBuffer ByteBuffer::subBuffer(SIZE pos) const
{
    if (pos >= m_size) return ByteBuffer();

    ByteBuffer ret(m_size - pos);
    memmove(ret.data(), m_bytes + pos, ret.size());

    return ret;
}
//=============================================================================
ByteBuffer& ByteBuffer::appendString(const char* str)
{
    if (*str == 0) return *this;

    const char* c = str;

    while (*c) c++;

    SIZE s = c - str;

    SIZE oldSize = m_size;

    _resize(s + m_size);

    if (m_bytes) memmove(m_bytes + oldSize, str, s);

    return *this;
}
//=============================================================================
ByteBuffer& ByteBuffer::appendChar(char c)
{
    SIZE s = m_size;

    s++;

    _resize(s);

    *(m_bytes + s - 1) = c;

    return *this;
}
//=============================================================================
cerberus::SIZE ByteBuffer::size() const { return m_size; }
//=============================================================================
bool ByteBuffer::isEmpty() const { return m_size == 0; }
//=============================================================================
ByteBuffer& ByteBuffer::resize(SIZE size)
{
    _resize(size);

    return *this;
}
//=============================================================================
ByteBuffer& ByteBuffer::assign(const ByteBuffer& other, SIZE len)
{
    if (m_bytes == other.m_bytes)  // same instance
    {
        return *this;
    }

    if (other.m_size == 0)
    {
        clear();
        return *this;
    }

    SIZE s;

    if (len)
        s = len;
    else
        s = other.m_size;

    _resize(s);

    memmove(m_bytes, other.m_bytes, s);

    return *this;
}
//=============================================================================
ByteBuffer& ByteBuffer::assign(const char* str)
{
    _clear();

    const char* c = str;

    while (*c) c++;

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

    return *this;
}
//=============================================================================
ByteBuffer& ByteBuffer::append(const ByteBuffer& other)
{
    SIZE s = other.m_size;

    if (s == 0) return *this;

    SIZE oldSize = m_size;
    SIZE newSize = oldSize + s;

    _resize(newSize);

    if (m_bytes) memmove(m_bytes + oldSize, other.m_bytes, s);

    return *this;
}
//=============================================================================
ByteBuffer& ByteBuffer::clear()
{
    _clear();
    return *this;
}
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
    std::string ret(m_size, 0);
    copyTo((BYTE*)ret.data(), m_size);
    return ret;
}
//=============================================================================
cerberus::OperationResult ByteBuffer::toNormalizedString() const
{
    auto str = toString();
    OperationResult res(OR_OK);
    res.i   = core::CerberusUtils::normalize(str);
    res.str = str;
    return res;
}
//=============================================================================
std::string ByteBuffer::toBinaryDumpString(uint32_t align) const
{
    std::string ret;
    uint32_t aligncounter = 0;

    for (SIZE i = 0; i < m_size; i++)
    {
        if (align && aligncounter == align)
        {
            ret.append("\n");
            aligncounter = 0;
        }

        ret.append(core::CerberusUtils::strPrint("%02hhx ", (uint8_t)(*(m_bytes + i))));

        aligncounter++;
    }

    return ret;
}
//=============================================================================
cerberus::OperationResult ByteBuffer::search(const char* str) const
{
    const char* c = str;

    while (*c) c++;

    SIZE s = c - str;

    if (s == 0 || s > m_size) return OR_WrongArgument;

    for (SIZE i = 0; i < m_size - s; i++)
    {
        for (SIZE j = 0; j < s; j++)
        {
            if (*(m_bytes + i + j) == *(str + j))
            {
                if (j == s - 1) return int64_t(i);
            }
            else
                break;
        }
    }

    return OR_NotFound;
}
//=============================================================================
bool ByteBuffer::startsWith(const ByteBuffer& buffer) const
{
    if (buffer.size() > size()) return false;

    for (SIZE i = 0; i < buffer.size(); i++)
    {
        if (getat(i) != buffer.getat(i)) return false;
    }

    return true;
}
//=============================================================================
bool ByteBuffer::endsWith(const ByteBuffer& buffer) const
{
    if (buffer.size() > size()) return false;

    for (SIZE i = 1; i <= buffer.size(); i++)
    {
        if (getat(size() - i) != buffer.getat(buffer.size() - i)) return false;
    }

    return true;
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
const ByteBuffer& ByteBuffer::consumeBlank() const
{
    while (!isEnd())
    {
        BYTE b = getat(m_pos);

        if (b != ' ' && b != 0x9 && b != '\n' && b != '\r')  // space, TAB, LF, CR
        {
            return *this;
        }

        m_pos++;
    }

    return *this;
}
//=============================================================================
ByteBuffer ByteBuffer::consumeUntil(const ByteBuffer& tokenSet) const
{
    ByteBuffer ret;

    while (!isEnd())
    {
        BYTE b = getat(m_pos);

        for (auto& el : tokenSet)
            if (b == el) return ret;

        m_pos++;
        ret += b;
    }

    return ret;
}
//=============================================================================
ByteBuffer ByteBuffer::read(SIZE len) const
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
cerberus::BYTE ByteBuffer::readByte() const
{
    auto b = at(m_pos);
    m_pos++;
    return b;
}
//=============================================================================
const ByteBuffer& ByteBuffer::seek(SIZE pos) const
{
    if (pos >= m_size)
        m_pos = m_size;
    else
        m_pos = pos;

    return *this;
}
//=============================================================================
cerberus::SIZE ByteBuffer::pos() const { return m_pos; }
//=============================================================================
const ByteBuffer& ByteBuffer::resetCursor(bool end) const
{
    if (end)
        m_pos = m_size;
    else
        m_pos = 0;

    return *this;
}
//=============================================================================
bool ByteBuffer::isEnd() const { return m_pos == m_size; }
//=============================================================================
const cerberus::BYTE& ByteBuffer::get() const { return at(m_pos); }
//=============================================================================
const ByteBuffer& ByteBuffer::moveCursor(OFFSET offset) const
{
    if (offset < 0)
    {
        if (llabs(offset) > m_pos)
            m_pos = 0;
        else
            m_pos = m_pos + offset;
    }
    else
    {
        if ((offset + m_pos) < m_size)
            m_pos = m_pos + offset;
        else
            m_pos = m_size;
    }

    return *this;
}
//=============================================================================
const ByteBuffer& ByteBuffer::next() const { return moveCursor(1); }
//=============================================================================
const ByteBuffer& ByteBuffer::prev() const { return moveCursor(-1); }
//=============================================================================
