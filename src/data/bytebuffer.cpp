#include "bytebuffer.h"

#include <cstring>

#include "src/exception/exceptioncatalog.h"
#include "src/mutex/mutex.h"
#include "src/mutex/mutexlocker.h"

using namespace cerberus::data;

//=============================================================================
void ByteBuffer::becomeOwner(bool force) const
{
    if (m_owner) return;

    if (!(*m_hasOwner))
    {
        *m_hasOwner = true;
        m_owner     = true;
        return;
    }

    if (!force)
    {
        return;
    }

    uint8_t* oldBuffer = m_bytes;
    SIZE oldSize       = *m_size;

    (*m_instances)--;

    m_bytes     = nullptr;           // null
    m_instances = new uint32_t(0);   // zero
    m_mutex     = new mutex::Mutex;  // new
    m_size      = new SIZE(0);       // zero
    m_hasOwner  = new bool(true);    // true

    if (oldSize != 0)
    {
        m_bytes = (uint8_t*)malloc(oldSize);
        if (m_bytes)
        {
            memmove(m_bytes, oldBuffer, oldSize);
            (*m_size) = oldSize;
            (*m_instances)++;
        }
    }
}
//=============================================================================
void ByteBuffer::_resize(SIZE size)
{
    if (size == 0)
    {
        _clear();
        return;
    }

    uint8_t* newbuf = (uint8_t*)realloc(m_bytes, size);

    if (newbuf == nullptr)
    {
        throw cerberusSystemExc("could not reallocate ByteBuffer memory");
    }

    m_bytes   = newbuf;
    (*m_size) = size;
}
//=============================================================================
void ByteBuffer::_clear()
{
    free(m_bytes);
    m_bytes   = nullptr;
    (*m_size) = 0;
}
//=============================================================================
ByteBuffer::ByteBuffer(SIZE size)
    : m_bytes(nullptr),
      m_instances(new uint32_t(0)),
      m_mutex(new mutex::Mutex),
      m_size(new SIZE(size)),
      m_hasOwner(new bool(true)),
      m_owner(true)
{
    if (size != 0)
    {
        m_bytes = (uint8_t*)malloc(size);

        if (m_bytes)
        {
            (*m_instances) = 1;
        }
    }
}
//=============================================================================
ByteBuffer::ByteBuffer(SIZE size, uint8_t val)
    : m_bytes(nullptr),
      m_instances(new uint32_t(0)),
      m_mutex(new mutex::Mutex),
      m_size(new SIZE(size)),
      m_hasOwner(new bool(true)),
      m_owner(true)
{
    if (size != 0)
    {
        m_bytes = (uint8_t*)malloc(size);
        if (m_bytes)
        {
            memset(m_bytes, val, size);
            (*m_instances) = 1;
        }
    }
}
//=============================================================================
ByteBuffer::ByteBuffer()
    : m_bytes(nullptr),
      m_instances(new uint32_t(0)),
      m_mutex(new mutex::Mutex),
      m_size(new SIZE(0)),
      m_hasOwner(new bool(true)),
      m_owner(true)
{
}
//=============================================================================
ByteBuffer::ByteBuffer(const ByteBuffer& other)
    : m_bytes(other.m_bytes),
      m_instances(other.m_instances),
      m_mutex(other.m_mutex),
      m_size(other.m_size),
      m_hasOwner(other.m_hasOwner),
      m_owner(false)
{
    m_mutex->lock();
    (*m_instances)++;
    m_mutex->unlock();
}
//=============================================================================
ByteBuffer::ByteBuffer(ByteBuffer&& other)
    : m_bytes(other.m_bytes),
      m_instances(other.m_instances),
      m_mutex(other.m_mutex),
      m_size(other.m_size),
      m_hasOwner(other.m_hasOwner),
      m_owner(true)
{
    other.m_bytes     = nullptr;
    other.m_instances = nullptr;
    other.m_mutex     = nullptr;
    other.m_size      = nullptr;
    other.m_hasOwner  = nullptr;

    if ((*m_instances) != 1)
    {
        throw cerberusIllegalStateExc("move constructor called on a still sharing ByteBuffer");
    }
}
//=============================================================================
ByteBuffer::ByteBuffer(const char* str)
    : m_bytes(nullptr),
      m_instances(new uint32_t(0)),
      m_mutex(new mutex::Mutex),
      m_size(new SIZE(0)),
      m_hasOwner(new bool(true)),
      m_owner(true)
{
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
            (*m_instances) = 1;
            (*m_size)      = s;
        }
    }
}
//=============================================================================
ByteBuffer::~ByteBuffer()
{
    if (!m_mutex) return;

    m_mutex->lock();
    if ((*m_instances) == 1)
    {
        delete m_mutex;
        delete m_instances;
        delete m_size;
        delete m_hasOwner;
        free(m_bytes);
    }
    else if ((*m_instances) != 0)
    {
        (*m_instances)--;
        if (m_owner)
        {
            (*m_hasOwner) = false;
        }
        m_mutex->unlock();
    }
}
//=============================================================================
unsigned char* ByteBuffer::data()
{
    mutex::MutexLocker ml(m_mutex);
    becomeOwner(true);
    return m_bytes;
}
//=============================================================================
const unsigned char* ByteBuffer::data() const
{
    mutex::MutexLocker ml(m_mutex);
    becomeOwner();
    return m_bytes;
}
//=============================================================================
unsigned char ByteBuffer::operator[](SIZE index)
{
    mutex::MutexLocker ml(m_mutex);
    becomeOwner();

    if (index >= *m_size)
    {
        throw cerberusIllegalArgExc("index out of bound, %u/%u", index, *m_size);
    }

    return *((unsigned char*)(m_bytes + index));
}
//=============================================================================
void ByteBuffer::appendFrom(const char *buffer, SIZE len)
{
    ByteBuffer buf(len);
    uint8_t* p = buf.data();
    memmove(p, buffer, len);

    append(buf);
}
//=============================================================================
void ByteBuffer::assignFrom(const char *buffer, SIZE len)
{
    clear();
    appendFrom(buffer, len);
}
//=============================================================================
void ByteBuffer::copyTo(char *buffer, SIZE maxLen)
{
    mutex::MutexLocker ml(m_mutex);
    becomeOwner();

    if(*m_size == 0 || !m_bytes)
    {
        return;
    }

    if(maxLen)
    {
        memmove(buffer, m_bytes, maxLen);
    }
    else
    {
        memmove(buffer, m_bytes, (*m_size));
    }
}
//=============================================================================
bool ByteBuffer::operator==(const ByteBuffer& other)
{
    mutex::MutexLocker ml1(m_mutex);
    becomeOwner();

    if (m_bytes == other.m_bytes)
    {
        return true;
    }

    mutex::MutexLocker ml2(other.m_mutex);

    if (m_bytes == nullptr || other.m_bytes == nullptr)
    {
        return false;
    }

    if (*m_size != *other.m_size)
    {
        return false;
    }

    return (memcmp(m_bytes, other.m_bytes, *m_size) == 0);
}
//=============================================================================
bool ByteBuffer::operator!=(const ByteBuffer& other) { return !(*this == other); }
//=============================================================================
void ByteBuffer::operator+=(const ByteBuffer& other) { append(other); }
//=============================================================================
void ByteBuffer::operator+=(unsigned char c)
{
    mutex::MutexLocker ml(m_mutex);

    becomeOwner(true);

    SIZE s = *m_size;

    s++;

    _resize(s);

    if (m_bytes)
    {
        *(m_bytes + s - 1) = c;
    }
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
ByteBuffer ByteBuffer::subBuffer(SIZE pos, SIZE len)
{
    mutex::MutexLocker ml(m_mutex);
    becomeOwner();

    ByteBuffer ret(len);
    uint8_t* d = ret.data();
    memmove(d, m_bytes + pos, len);

    return ret;
}
//=============================================================================
void ByteBuffer::appendString(const char* str)
{
    mutex::MutexLocker ml(m_mutex);
    becomeOwner(true);

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

    SIZE oldSize = *m_size;

    _resize(s + (*m_size));

    if (m_bytes)
    {
        memmove(m_bytes + oldSize, str, s);
    }
}
//=============================================================================
cerberus::SIZE ByteBuffer::size() const
{
    mutex::MutexLocker ml(m_mutex);
    becomeOwner();

    return *m_size;
}
//=============================================================================
void ByteBuffer::resize(SIZE size)
{
    if (size == 0)
    {
        clear();
        return;
    }

    mutex::MutexLocker ml(m_mutex);
    becomeOwner(true);
    _resize(size);
}
//=============================================================================
void ByteBuffer::assign(const ByteBuffer& buffer)
{
    mutex::MutexLocker ml2(buffer.m_mutex);

    if (*buffer.m_size == 0)
    {
        clear();
        return;
    }

    mutex::MutexLocker ml1(m_mutex);
    becomeOwner(true);

    SIZE s = *buffer.m_size;

    _resize(s);

    if (s)
    {
        memmove(m_bytes, buffer.m_bytes, s);
    }
}
//=============================================================================
void ByteBuffer::assign(const char* str)
{
    mutex::MutexLocker ml(m_mutex);

    becomeOwner(true);

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
            (*m_size) = s;
        }
    }
}
//=============================================================================
void ByteBuffer::append(const ByteBuffer& other)
{
    mutex::MutexLocker ml1(m_mutex);
    mutex::MutexLocker ml2(other.m_mutex);

    becomeOwner(true);

    SIZE s = *other.m_size;

    if (s == 0)
    {
        return;
    }

    SIZE oldSize = *m_size;
    SIZE newSize = oldSize + s;

    _resize(newSize);

    if (m_bytes)
    {
        memmove(m_bytes + oldSize, other.m_bytes, s);
    }
}
//=============================================================================
void ByteBuffer::clear()
{
    mutex::MutexLocker ml(m_mutex);
    becomeOwner(true);
    _clear();
}
//=============================================================================
bool ByteBuffer::isValid()
{
    mutex::MutexLocker ml(m_mutex);
    becomeOwner();
    return ((*m_size) != 0);
}
//=============================================================================
void ByteBuffer::appropriate()
{
    mutex::MutexLocker ml(m_mutex);
    becomeOwner(true);
}
//=============================================================================
uint32_t ByteBuffer::instances()
{
    mutex::MutexLocker ml(m_mutex);
    becomeOwner();
    return (*m_instances);
}
//=============================================================================
