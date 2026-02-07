#include "recipient.h"

#include "../message/message.h"  // IWYU pragma: export
#include "../thread/mutexlocker.h"

#include <algorithm>

using namespace crb;

//=============================================================================
Recipient::QueueState* Recipient::_q_nomutex(HASH32 channel_in)
{
    for (auto& q : m_queues)
        if (q.channel_in == channel_in) return &q;
    return nullptr;
}
//=============================================================================
const Recipient::QueueState* Recipient::_q_nomutex(HASH32 channel_in) const
{
    for (const auto& q : m_queues)
        if (q.channel_in == channel_in) return &q;
    return nullptr;
}
//=============================================================================
Recipient::QueueState* Recipient::_ensureQueue_nomutex(HASH32 channel_in)
{
    if (auto* q = _q_nomutex(channel_in)) return q;
    m_queues.emplace_back();
    auto& q = m_queues.back();
    q.channel_in = channel_in;
    return &q;
}
//=============================================================================
Recipient::RecipientChannel* Recipient::_channel_nomutex(HASH32 channel_out)
{
    for (auto& c : m_channels)
        if (c.channel_out == channel_out) return &c;
    return nullptr;
}
//=============================================================================
const Recipient::RecipientChannel* Recipient::_channel_nomutex(HASH32 channel_out) const
{
    for (const auto& c : m_channels)
        if (c.channel_out == channel_out) return &c;
    return nullptr;
}
//=============================================================================
Recipient::RecipientChannel* Recipient::_ensureChannel_nomutex(HASH32 channel_out)
{
    if (auto* c = _channel_nomutex(channel_out)) return c;
    m_channels.emplace_back();
    auto& c = m_channels.back();
    c.channel_out = channel_out;
    return &c;
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
    const SIZE c = (SIZE)q.queue.size() + q.reservedCount;

    if (q.queueLimitBytes && (q.queueBytes + q.reservedBytes + addBytes) > q.queueLimitBytes)
        return true;
    if (q.queueLimitCount && (c + addCount) > q.queueLimitCount) return true;

    return false;
}
//=============================================================================
void Recipient::_dropOldest_nomutex(QueueState& q, SIZE bytesToFit, SIZE countToFit)
{
    while (!q.queue.empty())
    {
        const bool bytesOk = (!q.queueLimitBytes) ||
            ((q.queueBytes + q.reservedBytes + bytesToFit) <= q.queueLimitBytes);
        const bool countOk =
            (!q.queueLimitCount) ||
            (((SIZE)q.queue.size() + q.reservedCount + countToFit) <= q.queueLimitCount);

        if (bytesOk && countOk) break;

        msg_ptr& front = q.queue.front();
        q.queueBytes -= front.memFootprint();
        q.queue.pop_front();
        ++q.dropped;
    }
}
//=============================================================================
bool Recipient::_consumeReserve_nomutex(QueueState& q, SIZE msgBytes)
{
    if (q.reservedCount == 0 || q.reservedBytes < msgBytes) return false;
    q.reservedCount -= 1;
    q.reservedBytes -= msgBytes;
    return true;
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
    m_queues.back().channel_in = 0;
}
//=============================================================================
Recipient::~Recipient()
{
    for (auto& q : m_queues) q.queue.clear();
    if (!m_extmutex) delete m_mutex;
}
//=============================================================================
void Recipient::setRecipient(Recipient* recipient, HASH32 channel_out, HASH32 channel_in)
{
    if (!recipient) return;

    MutexLocker loc(m_mutex);
    RecipientChannel* c = _ensureChannel_nomutex(channel_out);
    for (auto& entry : c->recipients)
    {
        if (entry.recipient != recipient) continue;
        entry.channel_in = channel_in;
        return;
    }

    c->recipients.push_back({recipient, channel_in});
}
//=============================================================================
void Recipient::removeRecipient(Recipient* recipient, HASH32 channel_out)
{
    if (!recipient) return;

    MutexLocker loc(m_mutex);
    for (auto it = m_channels.begin(); it != m_channels.end(); ++it)
    {
        if (it->channel_out != channel_out) continue;
        auto& list = it->recipients;
        list.erase(std::remove_if(list.begin(), list.end(),
                                  [recipient](const RecipientTarget& entry)
                                  { return entry.recipient == recipient; }),
                   list.end());
        if (list.empty()) m_channels.erase(it);
        return;
    }
}
//=============================================================================
msg_ptr Recipient::next(HASH32 channel_in)
{
    MutexLocker loc(m_mutex);
    QueueState* q = _q_nomutex(channel_in);
    if (!q) return msg_ptr();
    if (q->queue.empty()) return msg_ptr();

    msg_ptr m = std::move(q->queue.front());
    q->queue.pop_front();
    q->queueBytes -= m.memFootprint();
    return m;
}
//=============================================================================
msg_ptr Recipient::nextKeep(HASH32 channel_in) const
{
    MutexLocker loc(m_mutex);
    const QueueState* q = _q_nomutex(channel_in);
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
        q.reservedBytes = 0;
        q.reservedCount = 0;
    }
}
//=============================================================================
void Recipient::clear(HASH32 channel_in)
{
    MutexLocker loc(m_mutex);
    QueueState* q = _q_nomutex(channel_in);
    if (!q) return;
    q->queue.clear();
    q->queueBytes = 0;
    q->reservedBytes = 0;
    q->reservedCount = 0;
}
//=============================================================================
void Recipient::setQueueLimitBytes(SIZE bytes) { setQueueLimitBytes(bytes, 0); }
//=============================================================================
void Recipient::setQueueLimitCount(SIZE count) { setQueueLimitCount(count, 0); }
//=============================================================================
void Recipient::setOverflowPolicy(OverflowPolicy p) { setOverflowPolicy(p, 0); }
//=============================================================================
void Recipient::setQueueLimitBytes(SIZE bytes, HASH32 channel_in)
{
    MutexLocker loc(m_mutex);
    QueueState* q = _ensureQueue_nomutex(channel_in);
    q->queueLimitBytes = bytes;
}
//=============================================================================
void Recipient::setQueueLimitCount(SIZE count, HASH32 channel_in)
{
    MutexLocker loc(m_mutex);
    QueueState* q = _ensureQueue_nomutex(channel_in);
    q->queueLimitCount = count;
}
//=============================================================================
void Recipient::setOverflowPolicy(OverflowPolicy p, HASH32 channel_in)
{
    MutexLocker loc(m_mutex);
    QueueState* q = _ensureQueue_nomutex(channel_in);
    q->policy = p;
}
//=============================================================================
bool Recipient::reserve(msg_ptr& message, HASH32 channel_in)
{
    if (!message) return false;

    MutexLocker loc(m_mutex);
    QueueState* q = _ensureQueue_nomutex(channel_in);
    if (q->policy == DROP_OLDEST) return true;

    const SIZE msgBytes = message.memFootprint();
    if (_overLimit_nomutex(*q, msgBytes, 1))
        if (q->policy == DROP_OLDEST) _dropOldest_nomutex(*q, msgBytes, 1);

    if (_overLimit_nomutex(*q, msgBytes, 1)) return false;

    q->reservedBytes += msgBytes;
    q->reservedCount += 1;
    return true;
}
//=============================================================================
void Recipient::reserve_revert(msg_ptr& message, HASH32 channel_in)
{
    if (!message) return;

    const SIZE msgBytes = message.memFootprint();
    MutexLocker loc(m_mutex);
    QueueState* q = _q_nomutex(channel_in);
    if (!q) return;

    if (q->reservedCount > 0) q->reservedCount -= 1;
    if (q->reservedBytes >= msgBytes)
        q->reservedBytes -= msgBytes;
    else
        q->reservedBytes = 0;
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
void Recipient::resetStats(HASH32 channel_in)
{
    MutexLocker loc(m_mutex);

    QueueState* q = _ensureQueue_nomutex(channel_in);
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
OpRes Recipient::requeueFront(msg_ptr& message, HASH32 channel_in)
{
    if (!message) return OR_WrongArgument;

    bool first          = false;
    const SIZE msgBytes = message.memFootprint();

    {
        MutexLocker loc(m_mutex);
        QueueState* q = _ensureQueue_nomutex(channel_in);

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
OpRes Recipient::requeueBack(msg_ptr& message, HASH32 channel_in)
{
    if (!message) return OR_WrongArgument;

    bool first          = false;
    const SIZE msgBytes = message.memFootprint();

    {
        MutexLocker loc(m_mutex);
        QueueState* q = _ensureQueue_nomutex(channel_in);

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
SIZE Recipient::size_nomutex(HASH32 channel_in) const
{
    const QueueState* q = _q_nomutex(channel_in);
    if (!q) return 0;
    return (SIZE)q->queue.size();
}
//=============================================================================
OpRes Recipient::broadcast(msg_ptr& message, HASH32 channel_out)
{
    if (!message) return OR_Failure;

    MutexLocker loc(m_mutex);
    RecipientChannel* c = _channel_nomutex(channel_out);
    if (!c || c->recipients.empty()) return OR_NotFound;

    RecipientTarget* last = nullptr;
    for (auto it = c->recipients.rbegin(); it != c->recipients.rend(); ++it)
    {
        if (it->recipient)
        {
            last = &(*it);
            break;
        }
    }

    if (!last) return OR_NotFound;

    std::vector<RecipientTarget*> reserved;
    reserved.reserve(c->recipients.size());

    for (auto& target : c->recipients)
    {
        if (!target.recipient) continue;
        if (!target.recipient->reserve(message, target.channel_in))
        {
            for (auto* entry : reserved)
            {
                entry->recipient->reserve_revert(message, entry->channel_in);
            }
            return OR_Failure;
        }
        reserved.push_back(&target);
    }

    OpRes res = OR_OK;
    for (auto& target : c->recipients)
    {
        if (!target.recipient || &target == last) continue;
        OpRes r = target.recipient->send_deep(message, target.channel_in);
        if (r.fail()) res = OR_Failure;
    }

    OpRes r = last->recipient->send(message, last->channel_in);
    if (r.fail()) res = OR_Failure;

    return res;
}
//=============================================================================
OpRes Recipient::broadcast_deep(msg_ptr& message, HASH32 channel_out)
{
    if (!message) return OR_Failure;

    MutexLocker loc(m_mutex);
    RecipientChannel* c = _channel_nomutex(channel_out);
    if (!c || c->recipients.empty()) return OR_NotFound;

    std::vector<RecipientTarget*> reserved;
    reserved.reserve(c->recipients.size());

    for (auto& target : c->recipients)
    {
        if (!target.recipient) continue;
        if (!target.recipient->reserve(message, target.channel_in))
        {
            for (auto* entry : reserved)
            {
                entry->recipient->reserve_revert(message, entry->channel_in);
            }
            return OR_Failure;
        }
        reserved.push_back(&target);
    }

    OpRes res = OR_OK;
    for (auto& target : c->recipients)
    {
        if (!target.recipient) continue;
        OpRes r = target.recipient->send_deep(message, target.channel_in);
        if (r.fail()) res = OR_Failure;
    }

    return res;
}
//=============================================================================
OpRes Recipient::send(msg_ptr& message, HASH32 channel_in)
{
    if (!message) return OR_Failure;

    bool first = false;
    const SIZE msgBytes = message.memFootprint();

    {
        MutexLocker loc(m_mutex);
        QueueState* q = _ensureQueue_nomutex(channel_in);
        const bool reserved = _consumeReserve_nomutex(*q, msgBytes);

        if (!reserved)
        {
            if (_overLimit_nomutex(*q, msgBytes, 1))
                if (q->policy == DROP_OLDEST) _dropOldest_nomutex(*q, msgBytes, 1);

            if (_overLimit_nomutex(*q, msgBytes, 1))
            {
                if (!(q->policy == DROP_OLDEST && q->queue.empty()))
                {
                    ++q->rejected;
                    return OR_Failure;  // message untouched
                }
            }
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
OpRes Recipient::send_deep(const msg_ptr& src, HASH32 channel_in)
{
    if (!src) return OR_Failure;

    bool first = false;
    const SIZE estBytes = src.memFootprint();
    bool reserved = false;

    {
        MutexLocker loc(m_mutex);
        QueueState* q = _ensureQueue_nomutex(channel_in);
        reserved = _consumeReserve_nomutex(*q, estBytes);

        if (!reserved)
        {
            if (_overLimit_nomutex(*q, estBytes, 1))
                if (q->policy == DROP_OLDEST) _dropOldest_nomutex(*q, estBytes, 1);

            if (_overLimit_nomutex(*q, estBytes, 1))
            {
                if (!(q->policy == DROP_OLDEST && q->queue.empty()))
                {
                    ++q->rejected;
                    return OR_Failure;  // no duplicate
                }
            }
        }

        msg_ptr copy = src.duplicate();
        if (!copy)
        {
            if (reserved)
            {
                q->reservedBytes += estBytes;
                q->reservedCount += 1;
            }
            ++q->rejected;
            return OR_Failure;
        }

        const SIZE copyBytes = copy.memFootprint();

        if (!reserved)
        {
            if (_overLimit_nomutex(*q, copyBytes, 1))
                if (q->policy == DROP_OLDEST) _dropOldest_nomutex(*q, copyBytes, 1);

            if (_overLimit_nomutex(*q, copyBytes, 1))
            {
                if (!(q->policy == DROP_OLDEST && q->queue.empty()))
                {
                    ++q->rejected;
                    return OR_Failure;
                }
            }
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
OpRes Recipient::signal(HASH32 msgid, HASH32 channel_in)
{
    msg_ptr msg = Message::create(msgid);
    if (!msg) return OR_Failure;
    return send(msg, channel_in);
}
//=============================================================================
SIZE Recipient::size() const
{
    MutexLocker loc(m_mutex);
    return _totalCount_nomutex();
}
//=============================================================================
SIZE Recipient::size(HASH32 channel_in) const
{
    MutexLocker loc(m_mutex);
    const QueueState* q = _q_nomutex(channel_in);
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
bool Recipient::hasMessage(HASH32 channel_in) const
{
    MutexLocker loc(m_mutex);
    const QueueState* q = _q_nomutex(channel_in);
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
SIZE Recipient::getQueueBytesCount(HASH32 channel_in) const
{
    MutexLocker loc(m_mutex);
    const QueueState* q = _q_nomutex(channel_in);
    if (!q) return 0;
    return q->queueBytes;
}
//=============================================================================
SIZE Recipient::getQueueCount(HASH32 channel_in) const
{
    MutexLocker loc(m_mutex);
    const QueueState* q = _q_nomutex(channel_in);
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
SIZE Recipient::getDroppedCount(HASH32 channel_in) const
{
    MutexLocker loc(m_mutex);
    const QueueState* q = _q_nomutex(channel_in);
    if (!q) return 0;
    return q->dropped;
}
//=============================================================================
SIZE Recipient::getRejectedCount(HASH32 channel_in) const
{
    MutexLocker loc(m_mutex);
    const QueueState* q = _q_nomutex(channel_in);
    if (!q) return 0;
    return q->rejected;
}
//=============================================================================
