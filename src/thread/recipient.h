#ifndef CERBERUS_RECIPIENT_H
#define CERBERUS_RECIPIENT_H

#include <list>
#include <vector>

#include "../Cerberus_global.h"
#include "../message/message.h"
#include "../types.h"
#include "./mutex.h"

namespace crb
{
    class Recipient
    {
       public:
        enum OverflowPolicy
        {
            REJECT_NEW = 0,
            DROP_OLDEST
        };

       private:
        struct QueueState
        {
            HASH32 channel_in{0};
            std::list<msg_ptr> queue;

            LSIZE queueBytes{0};
            LSIZE queueLimitBytes{0};  // 0 = unlimited
            SIZE queueLimitCount{0};  // 0 = unlimited
            LSIZE reservedBytes{0};
            SIZE reservedCount{0};

            LSIZE peakBytes{0};
            SIZE peakCount{0};

            SIZE dropped{0};   // only used by DROP_OLDEST
            SIZE rejected{0};  // send()/send_deep() refused due to limits

            OverflowPolicy policy{REJECT_NEW};
        };

        std::list<QueueState> m_queues;

        struct RecipientTarget
        {
            Recipient* recipient{nullptr};
            HASH32 channel_in{0};
        };

        struct RecipientChannel
        {
            HASH32 channel_out{CRB_INVALID_ID};
            std::vector<RecipientTarget> recipients;
        };

        std::list<RecipientChannel> m_channels;

        // Total peaks across all queues
        LSIZE m_peakBytesTotal;
        SIZE m_peakCountTotal;

        mutable Mutex* m_mutex;
        bool m_extmutex;

        void _updatePeaks_nomutex(QueueState& q);
        void _updateTotalPeaks_nomutex();
        bool _overLimit_nomutex(const QueueState& q, LSIZE addBytes, SIZE addCount) const;
        void _dropOldest_nomutex(QueueState& q, LSIZE bytesToFit, SIZE countToFit);
        bool _consumeReserve_nomutex(QueueState& q, LSIZE msgBytes);

        QueueState* _q_nomutex(HASH32 channel_in);
        const QueueState* _q_nomutex(HASH32 channel_in) const;
        QueueState* _ensureQueue_nomutex(HASH32 channel_in);

        RecipientChannel* _channel_nomutex(HASH32 channel_out);
        const RecipientChannel* _channel_nomutex(HASH32 channel_out) const;
        RecipientChannel* _ensureChannel_nomutex(HASH32 channel_out);

        LSIZE _totalBytes_nomutex() const;
        SIZE _totalCount_nomutex() const;

        bool reserve(msg_ptr& message, HASH32 channel_in);
        void reserve_revert(msg_ptr& message, HASH32 channel_in);

       protected:
        CERBERUS_EXPORT Recipient(Mutex* mutex = nullptr);
        CERBERUS_EXPORT virtual ~Recipient();

        CERBERUS_EXPORT msg_ptr next(HASH32 channel_in = 0);
        CERBERUS_EXPORT msg_ptr nextKeep(HASH32 channel_in = 0) const;

        CERBERUS_EXPORT void clear();

        CERBERUS_EXPORT void clear(HASH32 channel_in);

        CERBERUS_EXPORT void setQueueLimitBytes(LSIZE bytes);  // 0 = unlimited
        CERBERUS_EXPORT void setQueueLimitCount(SIZE count);  // 0 = unlimited
        CERBERUS_EXPORT void setOverflowPolicy(OverflowPolicy p);

        CERBERUS_EXPORT void setQueueLimitBytes(LSIZE bytes, HASH32 channel_in);
        CERBERUS_EXPORT void setQueueLimitCount(SIZE count, HASH32 channel_in);
        CERBERUS_EXPORT void setOverflowPolicy(OverflowPolicy p, HASH32 channel_in);

        CERBERUS_EXPORT void resetStats();

        CERBERUS_EXPORT void resetStats(HASH32 channel_in);

        // Re-insert a message bypassing limits
        CERBERUS_EXPORT OpRes requeueFront(msg_ptr& message);  // consumes only on OR_OK
        CERBERUS_EXPORT OpRes requeueBack(msg_ptr& message);   // consumes only on OR_OK

        CERBERUS_EXPORT OpRes requeueFront(msg_ptr& message, HASH32 channel_in);
        CERBERUS_EXPORT OpRes requeueBack(msg_ptr& message, HASH32 channel_in);

        CERBERUS_EXPORT virtual void newMsg();
        CERBERUS_EXPORT virtual void newMsg_first();

        CERBERUS_EXPORT SIZE size_nomutex() const;

        CERBERUS_EXPORT SIZE size_nomutex(HASH32 channel_in) const;

        CERBERUS_EXPORT OpRes broadcast(msg_ptr& message, HASH32 channel_out = 0);
        CERBERUS_EXPORT OpRes broadcast_deep(msg_ptr& message, HASH32 channel_out = 0);

       public:
        CERBERUS_EXPORT void setRecipient(Recipient* recipient, HASH32 channel_out = 0, HASH32 channel_in = 0);
        CERBERUS_EXPORT void removeRecipient(Recipient* recipient, HASH32 channel_out = 0);
        // Consume only on success (OR_OK). On OR_Failure the message stays intact.
        CERBERUS_EXPORT OpRes send(msg_ptr& message, HASH32 channel_in = 0);

        // Deep-copy only if it can be accepted.
        CERBERUS_EXPORT OpRes send_deep(const msg_ptr& message, HASH32 channel_in = 0);

        // Create and send a message by id with no fields
        CERBERUS_EXPORT OpRes signal(HASH32 msgid, HASH32 channel_in = 0);

        CERBERUS_EXPORT SIZE size() const;
        CERBERUS_EXPORT SIZE size(HASH32 channel_in) const;

        CERBERUS_EXPORT bool hasMessage() const;
        CERBERUS_EXPORT bool hasMessage(HASH32 channel_in) const;

        CERBERUS_EXPORT LSIZE getQueueBytesCount() const;
        CERBERUS_EXPORT SIZE getQueueCount() const;

        CERBERUS_EXPORT LSIZE getQueueBytesCount(HASH32 channel_in) const;
        CERBERUS_EXPORT SIZE getQueueCount(HASH32 channel_in) const;

        CERBERUS_EXPORT LSIZE getPeakBytes() const;
        CERBERUS_EXPORT SIZE getPeakCount() const;

        CERBERUS_EXPORT SIZE getDroppedCount() const;
        CERBERUS_EXPORT SIZE getRejectedCount() const;

        CERBERUS_EXPORT SIZE getDroppedCount(HASH32 channel_in) const;
        CERBERUS_EXPORT SIZE getRejectedCount(HASH32 channel_in) const;
    };
}  // namespace crb

#endif  // CERBERUS_RECIPIENT_H
