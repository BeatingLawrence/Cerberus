#include "bytebuffer.h"

#include <boost/regex.hpp>
#include <cstdlib>
#include <cstring>

#include "src/cerberus.h"
#include "src/exception/exception.h"

using namespace cerberus;

//=============================================================================
void ByteBuffer::_resize(SIZE size)
{
    if (size == 0)
    {
        _clear();
        return;
    }

    BYTE* newbuf = (BYTE*)realloc(m_bytes, size + 1);

    if (newbuf == nullptr)
    {
        throw cSystemExc("could not allocate ByteBuffer memory");
    }

    m_bytes = newbuf;
    m_size  = size;
    m_pos   = 0;

    memset((m_bytes + size), 0, 1);  // the last byte is zero (string safety)
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
cerberus::BYTE& ByteBuffer::getat(SIZE index) const { return *((BYTE*)(m_bytes + index)); }
//=============================================================================
ByteBuffer::ByteBuffer(const void* buf, SIZE size)
    : m_bytes(nullptr),
      m_size(0),
      m_pos(0)
{
    assignFrom(buf, size);
}
//=============================================================================
ByteBuffer::ByteBuffer(SIZE size)
    : m_bytes(nullptr),
      m_size(size),
      m_pos(0)
{
    _resize(size);
}
//=============================================================================
ByteBuffer::ByteBuffer(SIZE size, BYTE val)
    : m_bytes(nullptr),
      m_size(size),
      m_pos(0)
{
    if (size != 0)
    {
        _resize(size);
        memset(m_bytes, val, size);
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

    while (*c) c++;

    SIZE s = c - str;

    if (s != 0)
    {
        _resize(s);
        memmove(m_bytes, str, s);
    }
}
//=============================================================================
ByteBuffer::ByteBuffer(const std::string& str)
    : m_bytes(nullptr),
      m_size(0),
      m_pos(0)
{
    if (str.empty()) return;

    SIZE s = str.size();

    _resize(s);

    if (s) memmove(m_bytes, str.data(), s);
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
const cerberus::BYTE* ByteBuffer::data(SIZE index) const { return m_bytes + index; }
//=============================================================================
cerberus::BYTE* ByteBuffer::data(SIZE index) { return m_bytes + index; }
//=============================================================================
const cerberus::BYTE& ByteBuffer::at(SIZE index) const
{
    if (index >= m_size)
    {
        throw cIllegalArgExc("index out of bound, %u/%u", index, m_size);
    }
    return getat(index);
}
//=============================================================================
cerberus::BYTE& ByteBuffer::at(SIZE index)
{
    if (index >= m_size)
    {
        throw cIllegalArgExc("index out of bound, %u/%u", index, m_size);
    }
    return getat(index);
}
//=============================================================================
cerberus::BYTE& ByteBuffer::operator[](SIZE index) { return at(index); }
//=============================================================================
const cerberus::BYTE& ByteBuffer::operator[](SIZE index) const { return at(index); }
//=============================================================================
ByteBuffer& ByteBuffer::appendFrom(const void* buffer, SIZE len)
{
    ByteBuffer buf(len);
    BYTE* p = buf.data();
    memmove(p, buffer, len);

    append(buf);

    return *this;
}
//=============================================================================
ByteBuffer& ByteBuffer::assignFrom(const void* buffer, SIZE len)
{
    clear();
    appendFrom(buffer, len);

    return *this;
}
//=============================================================================
const ByteBuffer& ByteBuffer::copyTo(void* buffer, SIZE maxLen) const
{
    if (m_size == 0 || !m_bytes) return *this;

    if (maxLen && maxLen <= m_size)
        memmove(buffer, m_bytes, maxLen);
    else
        memmove(buffer, m_bytes, m_size);

    return *this;
}
//=============================================================================
const ByteBuffer& ByteBuffer::copyFrom(void* buffer, SIZE len)
{
    if (m_size < len) _resize(len);

    memmove(m_bytes, buffer, len);

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
ByteBuffer ByteBuffer::trim(SIZE len) const
{
    ByteBuffer ret(len, 0);
    if (!m_bytes || !m_size || !len) return ret;

    copyTo(ret.data(), len);
    return std::move(ret);
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
ByteBuffer& ByteBuffer::appendString(const std::string str) { return appendString(str.c_str()); }
//=============================================================================
ByteBuffer& ByteBuffer::appendChar(char c)
{
    _resize(m_size + 1);

    if (m_size) memset((m_bytes + m_size - 1), c, 1);

    return *this;
}
//=============================================================================
ByteBuffer& ByteBuffer::append_2b(void* src) { return appendFrom((const BYTE*)src, 2); }
//=============================================================================
ByteBuffer& ByteBuffer::append_4b(void* src) { return appendFrom((const BYTE*)src, 4); }
//=============================================================================
ByteBuffer& ByteBuffer::append_8b(void* src) { return appendFrom((const BYTE*)src, 8); }
//=============================================================================
ByteBuffer& ByteBuffer::append_16b(void* src) { return appendFrom((const BYTE*)src, 16); }
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
        _resize(s);
        memmove(m_bytes, str, s);
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
StringOpRes ByteBuffer::toNormalizedString() const
{
    auto str = toString();
    CerberusUtils::normalize(str);
    return str;
}
//=============================================================================
std::string ByteBuffer::toBinaryDump(uint32_t align) const
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

        ret.append(CerberusUtils::strPrint("%02hhx ", (uint8_t)(*(m_bytes + i))));

        aligncounter++;
    }

    return ret;
}
//=============================================================================
std::string ByteBuffer::toHex() const { return CerberusUtils::hex(*this); }
//=============================================================================
IntOpRes ByteBuffer::search(const char* str) const
{
    if (m_size == 0) return OR_BadConditions;

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
IntOpRes ByteBuffer::locate(const ByteBuffer& match)
{
    if (match.isEmpty()) return OR_WrongArgument;
    if (isEmpty()) return OR_NotFound;

    SIZE i = 0, ret = 0;

    while (!isEnd())
    {
        if (getat(m_pos) == match.at(i))
        {
            if (i == 0) ret = m_pos;  // backup pos of first char

            i++;
            next();

            if (i == match.size()) return ret;
        }
        else if (i)
            i = 0;
        else
            next();
    }

    return OR_NotFound;
}
//=============================================================================
OpRes ByteBuffer::replace(const ByteBuffer& match, const ByteBuffer& replace)
{
    ByteBuffer temp;
    resetCursor();

    auto m = locate(match);
    if (m.fail()) return OR_NotFound;

    temp.appendFrom(data(), m.value);
    temp.append(replace);

    SIZE old;

    while (true)
    {
        old = m_pos;
        m   = locate(match);
        if (m.fail()) break;

        temp.appendFrom(data(old), m.value - old);
        temp.append(replace);
    }

    temp.appendFrom(data(old), m_pos - old);

    assign(temp);

    return OR_OK;
}
//=============================================================================
std::string ByteBuffer::getLine() const
{
    std::string ret;

    char c    = 0;
    char prev = 0;

    while (!isEnd())
    {
        c = getat(m_pos);

        if (c == '\n')
        {
            if (prev == '\r')
            {
                ret.pop_back();  // remove the \r
            }

            next();
            break;
        }

        ret.push_back(c);
        next();
        prev = c;
    }

    return ret;
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
OpResData<ByteBuffer> ByteBuffer::consumeUntil(const std::string& regex) const
{
    if (m_size == 0) return ByteBuffer();

    ByteBuffer ret;

    try
    {
        boost::regex r(regex);
        boost::cmatch m;
        std::string s;

        try
        {
            if (boost::regex_search((const char*)(m_bytes + m_pos), m, r))
            {
                ret   = subBuffer(m_pos, m.position());
                m_pos = m_pos + m.position();
            }
            else
                return OR_NotFound;
        }
        catch (std::regex_error& e)
        {
            return {OR_Failure, e.what()};
        }
    }
    catch (std::regex_error& e)
    {
        return {OR_WrongArgument, e.what()};
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
