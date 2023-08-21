#include "sharedbytebuffer.h"

#include <cstring>

#include "src/data/bytebuffer.h"
#include "src/mutex/mutex.h"
#include "src/mutex/mutexlocker.h"

using namespace cerberus::data;

//=============================================================================
void SharedByteBuffer::becomeOwner(bool force) const
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

    (*m_instances)--;

    m_buffer    = new ByteBuffer(*m_buffer);    // null
    m_instances = new uint32_t(1);              // zero
    m_mutex     = new mutex::Mutex(Recursive);  // new
    m_hasOwner  = new bool(true);               // true
    m_owner     = true;
}
//=============================================================================
SharedByteBuffer::SharedByteBuffer(SIZE size)
    : m_buffer(new ByteBuffer(size)),
      m_instances(new uint32_t(1)),
      m_mutex(new mutex::Mutex(Recursive)),
      m_hasOwner(new bool(true)),
      m_owner(true)
{
}
//=============================================================================
SharedByteBuffer::SharedByteBuffer(SIZE size, uint8_t val)
    : m_buffer(new ByteBuffer(size, val)),
      m_instances(new uint32_t(1)),
      m_mutex(new mutex::Mutex(Recursive)),
      m_hasOwner(new bool(true)),
      m_owner(true)
{
}
//=============================================================================
SharedByteBuffer::SharedByteBuffer(const ByteBuffer& buffer)
    : m_buffer(new ByteBuffer(buffer.m_bytes, buffer.m_size)),  // private constructor
      m_instances(new uint32_t(1)),
      m_mutex(new mutex::Mutex(Recursive)),
      m_hasOwner(new bool(true)),
      m_owner(true)
{
}
//=============================================================================
SharedByteBuffer::SharedByteBuffer()
    : m_buffer(new ByteBuffer),
      m_instances(new uint32_t(1)),
      m_mutex(new mutex::Mutex(Recursive)),
      m_hasOwner(new bool(true)),
      m_owner(true)
{
}
//=============================================================================
SharedByteBuffer::SharedByteBuffer(const SharedByteBuffer& other)
    : m_buffer(other.m_buffer),
      m_instances(other.m_instances),
      m_mutex(other.m_mutex),
      m_hasOwner(other.m_hasOwner),
      m_owner(false)
{
    m_mutex->lock();
    (*m_instances)++;
    m_mutex->unlock();
}
//=============================================================================
SharedByteBuffer::SharedByteBuffer(SharedByteBuffer&& other)
    : m_buffer(other.m_buffer),
      m_instances(other.m_instances),
      m_mutex(other.m_mutex),
      m_hasOwner(other.m_hasOwner),
      m_owner(other.m_owner)
{
    other.m_buffer    = nullptr;
    other.m_instances = nullptr;
    other.m_mutex     = nullptr;
    other.m_hasOwner  = nullptr;
    other.m_owner     = false;
}
//=============================================================================
SharedByteBuffer::SharedByteBuffer(const char* str)
    : m_buffer(new ByteBuffer(str)),
      m_instances(new uint32_t(1)),
      m_mutex(new mutex::Mutex(Recursive)),
      m_hasOwner(new bool(true)),
      m_owner(true)
{
}
//=============================================================================
SharedByteBuffer::~SharedByteBuffer()
{
    if (!m_mutex) return;

    m_mutex->lock();
    if ((*m_instances) == 1)
    {
        delete m_buffer;
        delete m_mutex;
        delete m_instances;
        delete m_hasOwner;
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
cerberus::BYTE* SharedByteBuffer::data()
{
    mutex::MutexLocker ml(m_mutex);
    becomeOwner(true);
    return m_buffer->data();
}
//=============================================================================
const cerberus::BYTE* SharedByteBuffer::data() const
{
    mutex::MutexLocker ml(m_mutex);
    becomeOwner();
    return m_buffer->data();
}
//=============================================================================
void SharedByteBuffer::appendFrom(const BYTE* buffer, SIZE len)
{
    mutex::MutexLocker ml(m_mutex);
    becomeOwner(true);

    m_buffer->appendFrom(buffer, len);
}
//=============================================================================
void SharedByteBuffer::assignFrom(const BYTE* buffer, SIZE len)
{
    mutex::MutexLocker ml(m_mutex);
    becomeOwner(true);

    m_buffer->assignFrom(buffer, len);
}
//=============================================================================
void SharedByteBuffer::copyTo(BYTE* buffer, SIZE maxLen)
{
    mutex::MutexLocker ml(m_mutex);
    becomeOwner();

    m_buffer->copyTo(buffer, maxLen);
}
//=============================================================================
bool SharedByteBuffer::operator==(const SharedByteBuffer& other)
{
    mutex::MutexLocker ml1(m_mutex);
    becomeOwner();

    return (*m_buffer) == (*other.m_buffer);
}
//=============================================================================
bool SharedByteBuffer::operator!=(const SharedByteBuffer& other) { return !(*this == other); }
//=============================================================================
void SharedByteBuffer::operator+=(const SharedByteBuffer& other) { append(other); }
//=============================================================================
void SharedByteBuffer::operator+=(unsigned char c)
{
    mutex::MutexLocker ml(m_mutex);
    becomeOwner(true);

    m_buffer->appendChar(c);
}
//=============================================================================
SharedByteBuffer& SharedByteBuffer::operator=(const SharedByteBuffer& other)
{
    assign(other);
    return *this;
}
//=============================================================================
SharedByteBuffer& SharedByteBuffer::operator=(const char* str)
{
    assign(str);
    return *this;
}
//=============================================================================
SharedByteBuffer SharedByteBuffer::subBuffer(SIZE pos, SIZE len)
{
    mutex::MutexLocker ml(m_mutex);
    becomeOwner();

    if ((pos + len) > m_buffer->size())
    {
        return SharedByteBuffer();  // null
    }

    return SharedByteBuffer(m_buffer->subBuffer(pos, len));
}
//=============================================================================
void SharedByteBuffer::appendString(const char* str)
{
    mutex::MutexLocker ml(m_mutex);
    becomeOwner(true);

    m_buffer->appendString(str);
}
//=============================================================================
cerberus::SIZE SharedByteBuffer::size() const
{
    mutex::MutexLocker ml(m_mutex);
    becomeOwner();

    return m_buffer->size();
}
//=============================================================================
void SharedByteBuffer::resize(SIZE size)
{
    mutex::MutexLocker ml(m_mutex);
    becomeOwner(true);
    m_buffer->resize(size);
}
//=============================================================================
void SharedByteBuffer::assign(const SharedByteBuffer& other, SIZE len)
{
    mutex::MutexLocker ml1(m_mutex);
    becomeOwner(true);
    mutex::MutexLocker ml2(other.m_mutex);

    m_buffer->assign(*other.m_buffer, len);
}
//=============================================================================
void SharedByteBuffer::assign(const char* str)
{
    mutex::MutexLocker ml(m_mutex);
    becomeOwner(true);

    m_buffer->assign(str);
}
//=============================================================================
void SharedByteBuffer::append(const SharedByteBuffer& other)
{
    mutex::MutexLocker ml1(m_mutex);
    becomeOwner(true);
    mutex::MutexLocker ml2(other.m_mutex);

    m_buffer->append(*other.m_buffer);
}
//=============================================================================
void SharedByteBuffer::clear()
{
    mutex::MutexLocker ml(m_mutex);
    becomeOwner(true);
    m_buffer->clear();
}
//=============================================================================
bool SharedByteBuffer::isValid()
{
    mutex::MutexLocker ml(m_mutex);
    becomeOwner();
    return m_buffer->isValid();
}
//=============================================================================
void SharedByteBuffer::appropriate()
{
    mutex::MutexLocker ml(m_mutex);
    becomeOwner(true);
}
//=============================================================================
uint32_t SharedByteBuffer::instances()
{
    mutex::MutexLocker ml(m_mutex);
    becomeOwner();
    return (*m_instances);
}
//=============================================================================
