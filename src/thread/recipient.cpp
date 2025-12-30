#include "recipient.h"

#include "../cerberus.h"
#include "../core/cerberuslog.h"
#include "../message/message.h"  // IWYU pragma: export
#include "../thread/mutexlocker.h"

using namespace cerberus;

//=============================================================================
void Recipient::_updatePeaks_nomutex()
{
    const SIZE c = (SIZE)m_queue.size();
    if (m_queueBytes > m_peakBytes) m_peakBytes = m_queueBytes;
    if (c > m_peakCount) m_peakCount = c;
}
//=============================================================================
void Recipient::_checkWarnings_nomutex()
{
    if (m_queueWarningBytes)
    {
        const bool over = (m_queueBytes > m_queueWarningBytes);
        if (over && !m_warnedBytes)
        {
            m_warnedBytes = true;
            logWarning("Queue bytes warning %u/%u", m_queueBytes, m_queueWarningBytes);
        }
        else if (!over)
            m_warnedBytes = false;
    }

    if (m_queueWarningCount)
    {
        const SIZE c    = (SIZE)m_queue.size();
        const bool over = (c > m_queueWarningCount);
        if (over && !m_warnedCount)
        {
            m_warnedCount = true;
            logWarning("Queue count warning %u/%u", c, m_queueWarningCount);
        }
        else if (!over)
            m_warnedCount = false;
    }
}
//=============================================================================
bool Recipient::_overLimit_nomutex(SIZE addBytes, SIZE addCount) const
{
    const SIZE c = (SIZE)m_queue.size();

    if (m_queueLimitBytes && (m_queueBytes + addBytes) > m_queueLimitBytes) return true;
    if (m_queueLimitCount && (c + addCount) > m_queueLimitCount) return true;

    return false;
}
//=============================================================================
void Recipient::_dropOldest_nomutex(SIZE bytesToFit, SIZE countToFit)
{
    while (!m_queue.empty())
    {
        const bool bytesOk = (!m_queueLimitBytes) || ((m_queueBytes + bytesToFit) <= m_queueLimitBytes);
        const bool countOk =
            (!m_queueLimitCount) || (((SIZE)m_queue.size() + countToFit) <= m_queueLimitCount);

        if (bytesOk && countOk) break;

        msg_ptr& front = m_queue.front();
        m_queueBytes -= front.memFootprint();
        m_queue.pop_front();
        ++m_dropped;
    }
}
//=============================================================================
msg_ptr Recipient::next()
{
    MutexLocker loc(m_mutex);
    if (m_queue.empty()) return msg_ptr();

    msg_ptr m = std::move(m_queue.front());
    m_queue.pop_front();
    m_queueBytes -= m.memFootprint();

    _checkWarnings_nomutex();
    return m;
}
//=============================================================================
msg_ptr Recipient::nextKeep() const
{
    MutexLocker loc(m_mutex);
    if (m_queue.empty()) return msg_ptr();

    return m_queue.front().duplicate();
}
//=============================================================================
void Recipient::clear()
{
    MutexLocker loc(m_mutex);
    m_queue.clear();
    m_queueBytes  = 0;
    m_warnedBytes = false;
    m_warnedCount = false;
}
//=============================================================================
void Recipient::setQueueWarning(SIZE bytes) { setQueueWarningBytes(bytes); }
//=============================================================================
void Recipient::setQueueWarningBytes(SIZE bytes)
{
    MutexLocker loc(m_mutex);
    m_queueWarningBytes = bytes;
    m_warnedBytes       = false;
}
//=============================================================================
void Recipient::setQueueWarningCount(SIZE count)
{
    MutexLocker loc(m_mutex);
    m_queueWarningCount = count;
    m_warnedCount       = false;
}
//=============================================================================
void Recipient::setQueueLimitBytes(SIZE bytes)
{
    MutexLocker loc(m_mutex);
    m_queueLimitBytes = bytes;
}
//=============================================================================
void Recipient::setQueueLimitCount(SIZE count)
{
    MutexLocker loc(m_mutex);
    m_queueLimitCount = count;
}
//=============================================================================
void Recipient::setOverflowPolicy(OverflowPolicy p)
{
    MutexLocker loc(m_mutex);
    m_policy = p;
}
//=============================================================================
void Recipient::resetStats()
{
    MutexLocker loc(m_mutex);
    m_peakBytes = 0;
    m_peakCount = 0;
    m_dropped   = 0;
    m_rejected  = 0;
}
//=============================================================================
void Recipient::newMsg() {}
//=============================================================================
void Recipient::newMsg_first() {}
//=============================================================================
SIZE Recipient::size_nomutex() const { return (SIZE)m_queue.size(); }
//=============================================================================
Recipient::Recipient(Mutex* mutex)
    : m_queueBytes(0),
      m_queueWarningBytes(0),
      m_queueLimitBytes(0),
      m_queueWarningCount(0),
      m_queueLimitCount(0),
      m_peakBytes(0),
      m_peakCount(0),
      m_dropped(0),
      m_rejected(0),
      m_warnedBytes(false),
      m_warnedCount(false),
      m_policy(REJECT_NEW),
      m_mutex(mutex),
      m_extmutex(false)
{
    if (m_mutex)
        m_extmutex = true;
    else
        m_mutex = new Mutex();
}
//=============================================================================
Recipient::~Recipient()
{
    m_queue.clear();
    if (!m_extmutex) delete m_mutex;
}
//=============================================================================
OpRes Recipient::send(msg_ptr& message)
{
    if (!message) return OR_Failure;

    bool first = false;

    {
        MutexLocker loc(m_mutex);

        const SIZE msgBytes = message.memFootprint();

        if (_overLimit_nomutex(msgBytes, 1))
        {
            if (m_policy == DROP_OLDEST) _dropOldest_nomutex(msgBytes, 1);
        }

        if (_overLimit_nomutex(msgBytes, 1))
        {
            ++m_rejected;
            return OR_Failure;  // message untouched
        }

        first = m_queue.empty();

        m_queueBytes += msgBytes;
        m_queue.push_back(std::move(message));  // consumed only here

        _updatePeaks_nomutex();
        _checkWarnings_nomutex();
    }

    if (first) newMsg_first();
    newMsg();
    return OR_OK;
}
//=============================================================================
OpRes Recipient::send_deep(const msg_ptr& src, HASH32 recipientOverride)
{
    if (!src) return OR_Failure;

    bool first = false;

    {
        MutexLocker loc(m_mutex);

        const SIZE estBytes = src.memFootprint();

        if (_overLimit_nomutex(estBytes, 1))
            if (m_policy == DROP_OLDEST) _dropOldest_nomutex(estBytes, 1);

        if (_overLimit_nomutex(estBytes, 1))
        {
            ++m_rejected;
            return OR_Failure;  // no duplicate
        }

        msg_ptr copy = src.duplicate();
        if (!copy)
        {
            ++m_rejected;
            return OR_Failure;
        }

        if (recipientOverride != CERBERUS_INVALID_ID) copy->setRecipient(recipientOverride);

        const SIZE copyBytes = copy.memFootprint();

        if (_overLimit_nomutex(copyBytes, 1))
            if (m_policy == DROP_OLDEST) _dropOldest_nomutex(copyBytes, 1);

        if (_overLimit_nomutex(copyBytes, 1))
        {
            ++m_rejected;
            return OR_Failure;
        }

        first = m_queue.empty();
        m_queueBytes += copyBytes;
        m_queue.push_back(std::move(copy));

        _updatePeaks_nomutex();
        _checkWarnings_nomutex();
    }

    if (first) newMsg_first();
    newMsg();
    return OR_OK;
}
//=============================================================================
SIZE Recipient::size() const
{
    MutexLocker loc(m_mutex);
    return (SIZE)m_queue.size();
}
//=============================================================================
bool Recipient::hasMessage() const
{
    MutexLocker loc(m_mutex);
    return !m_queue.empty();
}
//=============================================================================
SIZE Recipient::getQueueBytesCount() const
{
    MutexLocker loc(m_mutex);
    return m_queueBytes;
}
//=============================================================================
SIZE Recipient::getQueueCount() const
{
    MutexLocker loc(m_mutex);
    return (SIZE)m_queue.size();
}
//=============================================================================
SIZE Recipient::getPeakBytes() const
{
    MutexLocker loc(m_mutex);
    return m_peakBytes;
}
//=============================================================================
SIZE Recipient::getPeakCount() const
{
    MutexLocker loc(m_mutex);
    return m_peakCount;
}
//=============================================================================
SIZE Recipient::getDroppedCount() const
{
    MutexLocker loc(m_mutex);
    return m_dropped;
}
//=============================================================================
SIZE Recipient::getRejectedCount() const
{
    MutexLocker loc(m_mutex);
    return m_rejected;
}
//=============================================================================
