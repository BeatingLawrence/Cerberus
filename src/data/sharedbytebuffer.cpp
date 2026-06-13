#include "sharedbytebuffer.h"

#include "bytebuffer.h"
#include "src/data/bytebuffer.h"
#include "src/thread/mutex.h"
#include "src/thread/mutexlocker.h"

using namespace crb;

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

    m_buffer    = new ByteBuffer(*m_buffer);  // null
    m_instances = new uint32_t(1);            // zero
    m_mutex     = new Mutex(Recursive);       // new
    m_hasOwner  = new bool(true);             // true
    m_owner     = true;
}
//=============================================================================
SharedByteBuffer::SharedByteBuffer(LSIZE size)
    : m_buffer(new ByteBuffer(size)),
      m_instances(new uint32_t(1)),
      m_mutex(new Mutex(Recursive)),
      m_hasOwner(new bool(true)),
      m_owner(true)
{
}
//=============================================================================
SharedByteBuffer::SharedByteBuffer(LSIZE size, uint8_t val)
    : m_buffer(new ByteBuffer(size, val)),
      m_instances(new uint32_t(1)),
      m_mutex(new Mutex(Recursive)),
      m_hasOwner(new bool(true)),
      m_owner(true)
{
}
//=============================================================================
SharedByteBuffer::SharedByteBuffer(ByteBuffer& buffer)
    : m_buffer(new ByteBuffer),
      m_instances(new uint32_t(1)),
      m_mutex(new Mutex(Recursive)),
      m_hasOwner(new bool(true)),
      m_owner(true)
{
    m_buffer->m_bytes = buffer.m_bytes;  // private of ByteBuffer
    m_buffer->m_size  = buffer.m_size;

    buffer.m_bytes = nullptr;  // private of ByteBuffer
    buffer.m_size  = 0;
}
//=============================================================================
SharedByteBuffer::SharedByteBuffer()
    : m_buffer(new ByteBuffer),
      m_instances(new uint32_t(1)),
      m_mutex(new Mutex(Recursive)),
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
      m_mutex(new Mutex(Recursive)),
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
crb::BYTE* SharedByteBuffer::data()
{
    MutexLocker ml(m_mutex);
    becomeOwner(true);
    return m_buffer->data();
}
//=============================================================================
const crb::BYTE* SharedByteBuffer::data() const
{
    MutexLocker ml(m_mutex);
    becomeOwner();
    return m_buffer->data();
}
//=============================================================================
void SharedByteBuffer::appendFrom(const BYTE* buffer, LSIZE len)
{
    MutexLocker ml(m_mutex);
    becomeOwner(true);

    m_buffer->appendFrom(buffer, len);
}
//=============================================================================
void SharedByteBuffer::assignFrom(const BYTE* buffer, LSIZE len)
{
    MutexLocker ml(m_mutex);
    becomeOwner(true);

    m_buffer->assignFrom(buffer, len);
}
//=============================================================================
void SharedByteBuffer::copyTo(BYTE* buffer, LSIZE maxLen)
{
    MutexLocker ml(m_mutex);
    becomeOwner();

    m_buffer->copyTo(buffer, maxLen);
}
//=============================================================================
bool SharedByteBuffer::operator==(const SharedByteBuffer& other)
{
    MutexLocker ml1(m_mutex);
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
    MutexLocker ml(m_mutex);
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
SharedByteBuffer SharedByteBuffer::subBuffer(LSIZE pos, LSIZE len)
{
    MutexLocker ml(m_mutex);
    becomeOwner();

    auto sub = m_buffer->subBuffer(pos, len);
    return SharedByteBuffer(sub);
}
//=============================================================================
void SharedByteBuffer::appendString(const char* str)
{
    MutexLocker ml(m_mutex);
    becomeOwner(true);

    m_buffer->appendString(str);
}
//=============================================================================
crb::LSIZE SharedByteBuffer::size() const
{
    MutexLocker ml(m_mutex);
    becomeOwner();

    return m_buffer->size();
}
//=============================================================================
void SharedByteBuffer::resize(LSIZE size)
{
    MutexLocker ml(m_mutex);
    becomeOwner(true);
    m_buffer->resize(size);
}
//=============================================================================
void SharedByteBuffer::assign(const SharedByteBuffer& other, LSIZE len)
{
    MutexLocker ml1(m_mutex);
    becomeOwner(true);
    MutexLocker ml2(other.m_mutex);

    m_buffer->assign(*other.m_buffer, len);
}
//=============================================================================
void SharedByteBuffer::assign(const char* str)
{
    MutexLocker ml(m_mutex);
    becomeOwner(true);

    m_buffer->assign(str);
}
//=============================================================================
void SharedByteBuffer::append(const SharedByteBuffer& other)
{
    MutexLocker ml1(m_mutex);
    becomeOwner(true);
    MutexLocker ml2(other.m_mutex);

    m_buffer->append(*other.m_buffer);
}
//=============================================================================
void SharedByteBuffer::clear()
{
    MutexLocker ml(m_mutex);
    becomeOwner(true);
    m_buffer->clear();
}
//=============================================================================
bool SharedByteBuffer::isValid()
{
    MutexLocker ml(m_mutex);
    becomeOwner();
    return m_buffer->isValid();
}
//=============================================================================
void SharedByteBuffer::appropriate()
{
    MutexLocker ml(m_mutex);
    becomeOwner(true);
}
//=============================================================================
uint32_t SharedByteBuffer::instances()
{
    MutexLocker ml(m_mutex);
    becomeOwner();
    return (*m_instances);
}
//=============================================================================
