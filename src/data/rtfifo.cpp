#include "rtfifo.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#ifdef LINUX_SYSTEM
#include <poll.h>
#include <sys/eventfd.h>
#include <sys/mman.h>
#include <unistd.h>
#endif

#include "../cerberus.h"
#include "../exception/exception.h"

using namespace crb;

//=============================================================================
RTFifo::RTFifo()
    : m_head(0),
      m_blocks(nullptr),
      m_meta(nullptr),
      m_cap(0),
      m_mask(0),
      m_blockBytes(0),
      m_blocksSize(0),
      m_metaSize(0),
      m_eventFd(-1),
      m_overflow(false),
      m_lockMemory(false),
      m_tail(0)
{
}
//=============================================================================
RTFifo::RTFifo(uint32_t capPow2, SIZE blockBytes, SIZE align, bool lockMemory) : RTFifo()
{
    init(capPow2, blockBytes, align, lockMemory);
}
//=============================================================================
RTFifo::~RTFifo()
{
#ifndef LINUX_SYSTEM
    return;
#else
    shutdown();
#endif
}
//=============================================================================
void RTFifo::init(uint32_t capPow2, SIZE blockBytes, SIZE align, bool lockMemory)
{
#ifndef LINUX_SYSTEM
    (void)capPow2;
    (void)blockBytes;
    (void)align;
    (void)lockMemory;
    throw cImplMissExc("RTFifo is supported only on Linux");
#else
    if (m_blocks || m_meta || m_eventFd >= 0)
        throw cIllegalStateExc("RTFifo: already initialized");

    if (capPow2 == 0 || (capPow2 & (capPow2 - 1)) != 0)
        throw cIllegalArgExc("RTFifo: capacity must be power of two");

    if (blockBytes == 0) throw cIllegalArgExc("RTFifo: block size must be > 0");

    if (align == 0) align = 64;

    m_cap        = capPow2;
    m_mask       = capPow2 - 1;
    m_blockBytes = blockBytes;

    m_blocksSize = static_cast<SIZE>(capPow2) * blockBytes;
    m_metaSize   = static_cast<SIZE>(capPow2) * static_cast<SIZE>(sizeof(Meta));

    int ret = posix_memalign((void**)&m_blocks, align, m_blocksSize);
    if (ret != 0)
        throw cSystemExc("RTFifo: posix_memalign blocks failed: %s", strerror(ret));

    ret = posix_memalign((void**)&m_meta, align, m_metaSize);
    if (ret != 0)
    {
        free(m_blocks);
        m_blocks = nullptr;
        throw cSystemExc("RTFifo: posix_memalign meta failed: %s", strerror(ret));
    }

    memset(m_blocks, 0, m_blocksSize);
    memset(m_meta, 0, m_metaSize);

    m_eventFd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (m_eventFd < 0)
    {
        free(m_blocks);
        free(m_meta);
        m_blocks = nullptr;
        m_meta   = nullptr;
        throw cSystemExc("RTFifo: eventfd failed: %s", strerror(errno));
    }

    m_lockMemory = lockMemory;
    if (m_lockMemory)
    {
        if (mlock(m_blocks, m_blocksSize) != 0)
        {
            shutdown();
            throw cSystemExc("RTFifo: mlock blocks failed: %s", strerror(errno));
        }

        if (mlock(m_meta, m_metaSize) != 0)
        {
            shutdown();
            throw cSystemExc("RTFifo: mlock meta failed: %s", strerror(errno));
        }
    }
#endif
}
//=============================================================================
void RTFifo::shutdown()
{
#ifndef LINUX_SYSTEM
    return;
#else
    if (m_eventFd >= 0)
    {
        close(m_eventFd);
        m_eventFd = -1;
    }

    if (m_lockMemory)
    {
        if (m_blocks) munlock(m_blocks, m_blocksSize);
        if (m_meta) munlock(m_meta, m_metaSize);
    }

    if (m_blocks)
    {
        free(m_blocks);
        m_blocks = nullptr;
    }

    if (m_meta)
    {
        free(m_meta);
        m_meta = nullptr;
    }

    m_cap        = 0;
    m_mask       = 0;
    m_blockBytes = 0;
    m_blocksSize = 0;
    m_metaSize   = 0;
    m_lockMemory = false;
    m_head.store(0, std::memory_order_relaxed);
    m_tail.store(0, std::memory_order_relaxed);
    m_overflow.store(false, std::memory_order_relaxed);
#endif
}
//=============================================================================
uint8_t* RTFifo::tryBeginWrite(uint32_t* outSlot)
{
#ifndef LINUX_SYSTEM
    (void)outSlot;
    return nullptr;
#else
    if (!m_blocks) return nullptr;
    const uint32_t head = m_head.load(std::memory_order_relaxed);
    const uint32_t tail = m_tail.load(std::memory_order_acquire);

    if ((uint32_t)(head - tail) >= m_cap)
    {
        m_overflow.store(true, std::memory_order_relaxed);
        return nullptr;
    }

    const uint32_t slot = head & m_mask;
    if (outSlot) *outSlot = slot;
    return m_blocks + (static_cast<size_t>(slot) * m_blockBytes);
#endif
}
//=============================================================================
void RTFifo::endWrite(uint32_t slot, SIZE len, uint64_t seq)
{
#ifndef LINUX_SYSTEM
    (void)slot;
    (void)len;
    (void)seq;
    return;
#else
    if (len > m_blockBytes) len = m_blockBytes;

    m_meta[slot].len = len;
    m_meta[slot].seq = seq;

    const uint32_t head = m_head.load(std::memory_order_relaxed);
    const uint32_t tail = m_tail.load(std::memory_order_acquire);
    const bool wasEmpty = (head == tail);

    m_head.store(head + 1, std::memory_order_release);

    if (wasEmpty)
    {
        if (eventfd_write(m_eventFd, 1) != 0)
        {
            logWarning("RTFifo: eventfd_write failed: %s", strerror(errno));
        }
    }
#endif
}
//=============================================================================
const uint8_t* RTFifo::tryBeginRead(uint32_t* outSlot, SIZE* outLen, uint64_t* outSeq) const
{
#ifndef LINUX_SYSTEM
    (void)outSlot;
    (void)outLen;
    (void)outSeq;
    return nullptr;
#else
    if (!m_blocks) return nullptr;
    const uint32_t tail = m_tail.load(std::memory_order_relaxed);
    const uint32_t head = m_head.load(std::memory_order_acquire);

    if (tail == head) return nullptr;

    const uint32_t slot = tail & m_mask;
    if (outSlot) *outSlot = slot;
    if (outLen) *outLen = m_meta[slot].len;
    if (outSeq) *outSeq = m_meta[slot].seq;

    return m_blocks + (static_cast<size_t>(slot) * m_blockBytes);
#endif
}
//=============================================================================
void RTFifo::endRead()
{
#ifndef LINUX_SYSTEM
    return;
#else
    m_tail.fetch_add(1, std::memory_order_release);
#endif
}
//=============================================================================
OpRes RTFifo::waitData(int timeoutMs) const
{
#ifndef LINUX_SYSTEM
    (void)timeoutMs;
    return OR_Unavailable;
#else
    if (m_eventFd < 0) return OR_Unavailable;
    const uint32_t tail = m_tail.load(std::memory_order_relaxed);
    const uint32_t head = m_head.load(std::memory_order_acquire);
    if (tail != head) return OR_OK;

    pollfd pfd{};
    pfd.fd     = m_eventFd;
    pfd.events = POLLIN;

    int ret;
    do
    {
        ret = poll(&pfd, 1, timeoutMs);
    } while (ret < 0 && errno == EINTR);

    if (ret == 0) return OR_TimedOut;
    if (ret < 0) return OR_SystemFailure;

    if (pfd.revents & POLLIN)
    {
        uint64_t v = 0;
        if (read(m_eventFd, &v, sizeof(v)) < 0 && errno != EAGAIN)
            return OR_SystemFailure;
        return OR_OK;
    }

    return OR_Failure;
#endif
}
//=============================================================================
