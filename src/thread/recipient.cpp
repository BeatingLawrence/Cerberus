#include "recipient.h"

#include "../message/message.h"  // IWYU pragma: export
#include "../thread/mutexlocker.h"

using namespace cerberus;

//=============================================================================
Recipient::QueueState* Recipient::_q_nomutex(SIZE queueIndex)
{
    for (auto& q : m_queues)
        if (q.index == queueIndex) return &q;
    return nullptr;
}
//=============================================================================
const Recipient::QueueState* Recipient::_q_nomutex(SIZE queueIndex) const
{
    for (const auto& q : m_queues)
        if (q.index == queueIndex) return &q;
    return nullptr;
}
//=============================================================================
Recipient::QueueState* Recipient::_ensureQueue_nomutex(SIZE queueIndex)
{
    if (auto* q = _q_nomutex(queueIndex)) return q;
    m_queues.emplace_back();
    auto& q = m_queues.back();
    q.index = queueIndex;
    return &q;
}
//=============================================================================
SIZE Recipient::_totalBytes_nomutex() const
{
    SIZE tot = 0;
    for (auto& q : m_queues) tot += q.queueBytes;
    return tot;
}
//=============================================================================
SIZE Recipient::_totalCount_nomutex() const
{
    SIZE tot = 0;
    for (auto& q : m_queues) tot += (SIZE)q.queue.size();
    return tot;
}
//=============================================================================
void Recipient::_updatePeaks_nomutex(QueueState& q)
{
    const SIZE c = (SIZE)q.queue.size();
    if (q.queueBytes > q.peakBytes) q.peakBytes = q.queueBytes;
    if (c > q.peakCount) q.peakCount = c;
}
//=============================================================================
void Recipient::_updateTotalPeaks_nomutex()
{
    const SIZE tb = _totalBytes_nomutex();
    const SIZE tc = _totalCount_nomutex();
    if (tb > m_peakBytesTotal) m_peakBytesTotal = tb;
    if (tc > m_peakCountTotal) m_peakCountTotal = tc;
}
//=============================================================================
bool Recipient::_overLimit_nomutex(const QueueState& q, SIZE addBytes, SIZE addCount) const
{
    const SIZE c = (SIZE)q.queue.size();

    if (q.queueLimitBytes && (q.queueBytes + addBytes) > q.queueLimitBytes) return true;
    if (q.queueLimitCount && (c + addCount) > q.queueLimitCount) return true;

    return false;
}
//=============================================================================
void Recipient::_dropOldest_nomutex(QueueState& q, SIZE bytesToFit, SIZE countToFit)
{
    while (!q.queue.empty())
    {
        const bool bytesOk = (!q.queueLimitBytes) || ((q.queueBytes + bytesToFit) <= q.queueLimitBytes);
        const bool countOk =
            (!q.queueLimitCount) || (((SIZE)q.queue.size() + countToFit) <= q.queueLimitCount);

        if (bytesOk && countOk) break;

        msg_ptr& front = q.queue.front();
        q.queueBytes -= front.memFootprint();
        q.queue.pop_front();
        ++q.dropped;
    }
}
//=============================================================================
Recipient::Recipient(Mutex* mutex)
    : m_peakBytesTotal(0),
      m_peakCountTotal(0),
      m_mutex(mutex),
      m_extmutex(false)
{
    if (m_mutex)
        m_extmutex = true;
    else
        m_mutex = new Mutex();

    m_queues.emplace_back();
    m_queues.back().index = 0;
}
//=============================================================================
Recipient::~Recipient()
{
    for (auto& q : m_queues) q.queue.clear();
    if (!m_extmutex) delete m_mutex;
}
msg_ptr Recipient::next() { return next(0); }
//=============================================================================
msg_ptr Recipient::nextKeep() const { return nextKeep(0); }
//=============================================================================
msg_ptr Recipient::next(SIZE queueIndex)
{
    MutexLocker loc(m_mutex);
    QueueState* q = _q_nomutex(queueIndex);
    if (!q) return msg_ptr();
    if (q->queue.empty()) return msg_ptr();

    msg_ptr m = std::move(q->queue.front());
    q->queue.pop_front();
    q->queueBytes -= m.memFootprint();
    return m;
}
//=============================================================================
msg_ptr Recipient::nextKeep(SIZE queueIndex) const
{
    MutexLocker loc(m_mutex);
    const QueueState* q = _q_nomutex(queueIndex);
    if (!q) return msg_ptr();
    if (q->queue.empty()) return msg_ptr();

    return q->queue.front().duplicate();
}
//=============================================================================
void Recipient::clear()
{
    MutexLocker loc(m_mutex);
    for (auto& q : m_queues)
    {
        q.queue.clear();
        q.queueBytes = 0;
    }
}
//=============================================================================
void Recipient::clear(SIZE queueIndex)
{
    MutexLocker loc(m_mutex);
    QueueState* q = _q_nomutex(queueIndex);
    if (!q) return;
    q->queue.clear();
    q->queueBytes = 0;
}
//=============================================================================
void Recipient::setQueueLimitBytes(SIZE bytes) { setQueueLimitBytes(bytes, 0); }
//=============================================================================
void Recipient::setQueueLimitCount(SIZE count) { setQueueLimitCount(count, 0); }
//=============================================================================
void Recipient::setOverflowPolicy(OverflowPolicy p) { setOverflowPolicy(p, 0); }
//=============================================================================
void Recipient::setQueueLimitBytes(SIZE bytes, SIZE queueIndex)
{
    MutexLocker loc(m_mutex);
    QueueState* q = _ensureQueue_nomutex(queueIndex);
    q->queueLimitBytes = bytes;
}
//=============================================================================
void Recipient::setQueueLimitCount(SIZE count, SIZE queueIndex)
{
    MutexLocker loc(m_mutex);
    QueueState* q = _ensureQueue_nomutex(queueIndex);
    q->queueLimitCount = count;
}
//=============================================================================
void Recipient::setOverflowPolicy(OverflowPolicy p, SIZE queueIndex)
{
    MutexLocker loc(m_mutex);
    QueueState* q = _ensureQueue_nomutex(queueIndex);
    q->policy = p;
}
//=============================================================================
void Recipient::resetStats()
{
    MutexLocker loc(m_mutex);

    for (auto& q : m_queues)
    {
        q.peakBytes = 0;
        q.peakCount = 0;
        q.dropped   = 0;
        q.rejected  = 0;
    }
    m_peakBytesTotal = 0;
    m_peakCountTotal = 0;
}
//=============================================================================
void Recipient::resetStats(SIZE queueIndex)
{
    MutexLocker loc(m_mutex);

    QueueState* q = _ensureQueue_nomutex(queueIndex);
    q->peakBytes = 0;
    q->peakCount = 0;
    q->dropped   = 0;
    q->rejected  = 0;
}
//=============================================================================
OpRes Recipient::requeueFront(msg_ptr& message) { return requeueFront(message, 0); }
//=============================================================================
OpRes Recipient::requeueBack(msg_ptr& message) { return requeueBack(message, 0); }
//=============================================================================
OpRes Recipient::requeueFront(msg_ptr& message, SIZE queueIndex)
{
    if (!message) return OR_WrongArgument;

    bool first          = false;
    const SIZE msgBytes = message.memFootprint();

    {
        MutexLocker loc(m_mutex);
        QueueState* q = _ensureQueue_nomutex(queueIndex);

        first = q->queue.empty();
        q->queueBytes += msgBytes;
        q->queue.push_front(std::move(message));

        _updatePeaks_nomutex(*q);
        _updateTotalPeaks_nomutex();
    }

    if (first) newMsg_first();
    newMsg();
    return OR_OK;
}
//=============================================================================
OpRes Recipient::requeueBack(msg_ptr& message, SIZE queueIndex)
{
    if (!message) return OR_WrongArgument;

    bool first          = false;
    const SIZE msgBytes = message.memFootprint();

    {
        MutexLocker loc(m_mutex);
        QueueState* q = _ensureQueue_nomutex(queueIndex);

        first = q->queue.empty();
        q->queueBytes += msgBytes;
        q->queue.push_back(std::move(message));

        _updatePeaks_nomutex(*q);
        _updateTotalPeaks_nomutex();
    }

    if (first) newMsg_first();
    newMsg();
    return OR_OK;
}
//=============================================================================
void Recipient::newMsg() {}
//=============================================================================
void Recipient::newMsg_first() {}
//=============================================================================
SIZE Recipient::size_nomutex() const
{
    // total count for all queues
    return _totalCount_nomutex();
}
//=============================================================================
SIZE Recipient::size_nomutex(SIZE queueIndex) const
{
    const QueueState* q = _q_nomutex(queueIndex);
    if (!q) return 0;
    return (SIZE)q->queue.size();
}
//=============================================================================
OpRes Recipient::send(msg_ptr& message) { return send(message, 0); }
//=============================================================================
OpRes Recipient::send(msg_ptr& message, SIZE queueIndex)
{
    if (!message) return OR_Failure;

    bool first = false;

    {
        MutexLocker loc(m_mutex);
        QueueState* q = _ensureQueue_nomutex(queueIndex);

        const SIZE msgBytes = message.memFootprint();

        if (_overLimit_nomutex(*q, msgBytes, 1))
            if (q->policy == DROP_OLDEST) _dropOldest_nomutex(*q, msgBytes, 1);

        if (_overLimit_nomutex(*q, msgBytes, 1))
        {
            ++q->rejected;
            return OR_Failure;  // message untouched
        }

        first = q->queue.empty();

        q->queueBytes += msgBytes;
        q->queue.push_back(std::move(message));  // consumed only here

        _updatePeaks_nomutex(*q);
        _updateTotalPeaks_nomutex();
    }

    if (first) newMsg_first();
    newMsg();
    return OR_OK;
}
//=============================================================================
OpRes Recipient::send_deep(const msg_ptr& src) { return send_deep(src, 0); }
//=============================================================================
OpRes Recipient::send_deep(const msg_ptr& src, SIZE queueIndex)
{
    if (!src) return OR_Failure;

    bool first = false;

    {
        MutexLocker loc(m_mutex);
        QueueState* q = _ensureQueue_nomutex(queueIndex);

        const SIZE estBytes = src.memFootprint();

        if (_overLimit_nomutex(*q, estBytes, 1))
            if (q->policy == DROP_OLDEST) _dropOldest_nomutex(*q, estBytes, 1);

        if (_overLimit_nomutex(*q, estBytes, 1))
        {
            ++q->rejected;
            return OR_Failure;  // no duplicate
        }

        msg_ptr copy = src.duplicate();
        if (!copy)
        {
            ++q->rejected;
            return OR_Failure;
        }

        const SIZE copyBytes = copy.memFootprint();

        if (_overLimit_nomutex(*q, copyBytes, 1))
            if (q->policy == DROP_OLDEST) _dropOldest_nomutex(*q, copyBytes, 1);

        if (_overLimit_nomutex(*q, copyBytes, 1))
        {
            ++q->rejected;
            return OR_Failure;
        }

        first = q->queue.empty();
        q->queueBytes += copyBytes;
        q->queue.push_back(std::move(copy));

        _updatePeaks_nomutex(*q);
        _updateTotalPeaks_nomutex();
    }

    if (first) newMsg_first();
    newMsg();
    return OR_OK;
}
//=============================================================================
SIZE Recipient::size() const
{
    MutexLocker loc(m_mutex);
    return _totalCount_nomutex();
}
//=============================================================================
SIZE Recipient::size(SIZE queueIndex) const
{
    MutexLocker loc(m_mutex);
    const QueueState* q = _q_nomutex(queueIndex);
    if (!q) return 0;
    return (SIZE)q->queue.size();
}
//=============================================================================
bool Recipient::hasMessage() const
{
    MutexLocker loc(m_mutex);
    for (auto& q : m_queues)
        if (!q.queue.empty()) return true;
    return false;
}
//=============================================================================
bool Recipient::hasMessage(SIZE queueIndex) const
{
    MutexLocker loc(m_mutex);
    const QueueState* q = _q_nomutex(queueIndex);
    if (!q) return false;
    return !q->queue.empty();
}
//=============================================================================
SIZE Recipient::getQueueBytesCount() const
{
    MutexLocker loc(m_mutex);
    return _totalBytes_nomutex();
}
//=============================================================================
SIZE Recipient::getQueueCount() const
{
    MutexLocker loc(m_mutex);
    return _totalCount_nomutex();
}
//=============================================================================
SIZE Recipient::getQueueBytesCount(SIZE queueIndex) const
{
    MutexLocker loc(m_mutex);
    const QueueState* q = _q_nomutex(queueIndex);
    if (!q) return 0;
    return q->queueBytes;
}
//=============================================================================
SIZE Recipient::getQueueCount(SIZE queueIndex) const
{
    MutexLocker loc(m_mutex);
    const QueueState* q = _q_nomutex(queueIndex);
    if (!q) return 0;
    return (SIZE)q->queue.size();
}
//=============================================================================
SIZE Recipient::getPeakBytes() const
{
    MutexLocker loc(m_mutex);
    return m_peakBytesTotal;
}
//=============================================================================
SIZE Recipient::getPeakCount() const
{
    MutexLocker loc(m_mutex);
    return m_peakCountTotal;
}
//=============================================================================
SIZE Recipient::getDroppedCount() const
{
    MutexLocker loc(m_mutex);
    SIZE tot = 0;
    for (auto& q : m_queues) tot += q.dropped;
    return tot;
}
//=============================================================================
SIZE Recipient::getRejectedCount() const
{
    MutexLocker loc(m_mutex);
    SIZE tot = 0;
    for (auto& q : m_queues) tot += q.rejected;
    return tot;
}
//=============================================================================
SIZE Recipient::getDroppedCount(SIZE queueIndex) const
{
    MutexLocker loc(m_mutex);
    const QueueState* q = _q_nomutex(queueIndex);
    if (!q) return 0;
    return q->dropped;
}
//=============================================================================
SIZE Recipient::getRejectedCount(SIZE queueIndex) const
{
    MutexLocker loc(m_mutex);
    const QueueState* q = _q_nomutex(queueIndex);
    if (!q) return 0;
    return q->rejected;
}
//=============================================================================
