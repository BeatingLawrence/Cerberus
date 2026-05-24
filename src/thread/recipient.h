#ifndef CERBERUS_RECIPIENT_H
#define CERBERUS_RECIPIENT_H

#include <list>
#include <vector>

#include "../Cerberus_global.h"
#include "../types.h"
#include "./mutex.h"

namespace crb
{
    class CERBERUS_EXPORT Recipient
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

            SIZE queueBytes{0};
            SIZE queueLimitBytes{0};  // 0 = unlimited
            SIZE queueLimitCount{0};  // 0 = unlimited
            SIZE reservedBytes{0};
            SIZE reservedCount{0};

            SIZE peakBytes{0};
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
        SIZE m_peakBytesTotal;
        SIZE m_peakCountTotal;

        mutable Mutex* m_mutex;
        bool m_extmutex;

        void _updatePeaks_nomutex(QueueState& q);
        void _updateTotalPeaks_nomutex();
        bool _overLimit_nomutex(const QueueState& q, SIZE addBytes, SIZE addCount) const;
        void _dropOldest_nomutex(QueueState& q, SIZE bytesToFit, SIZE countToFit);
        bool _consumeReserve_nomutex(QueueState& q, SIZE msgBytes);

        QueueState* _q_nomutex(HASH32 channel_in);
        const QueueState* _q_nomutex(HASH32 channel_in) const;
        QueueState* _ensureQueue_nomutex(HASH32 channel_in);

        RecipientChannel* _channel_nomutex(HASH32 channel_out);
        const RecipientChannel* _channel_nomutex(HASH32 channel_out) const;
        RecipientChannel* _ensureChannel_nomutex(HASH32 channel_out);

        SIZE _totalBytes_nomutex() const;
        SIZE _totalCount_nomutex() const;

        bool reserve(msg_ptr& message, HASH32 channel_in);
        void reserve_revert(msg_ptr& message, HASH32 channel_in);

       protected:
        Recipient(Mutex* mutex = nullptr);
        virtual ~Recipient();

        msg_ptr next(HASH32 channel_in = 0);
        msg_ptr nextKeep(HASH32 channel_in = 0) const;

        void clear();

        void clear(HASH32 channel_in);

        void setQueueLimitBytes(SIZE bytes);  // 0 = unlimited
        void setQueueLimitCount(SIZE count);  // 0 = unlimited
        void setOverflowPolicy(OverflowPolicy p);

        void setQueueLimitBytes(SIZE bytes, HASH32 channel_in);
        void setQueueLimitCount(SIZE count, HASH32 channel_in);
        void setOverflowPolicy(OverflowPolicy p, HASH32 channel_in);

        void resetStats();

        void resetStats(HASH32 channel_in);

        // Re-insert a message bypassing limits
        OpRes requeueFront(msg_ptr& message);  // consumes only on OR_OK
        OpRes requeueBack(msg_ptr& message);   // consumes only on OR_OK

        OpRes requeueFront(msg_ptr& message, HASH32 channel_in);
        OpRes requeueBack(msg_ptr& message, HASH32 channel_in);

        virtual void newMsg();
        virtual void newMsg_first();

        SIZE size_nomutex() const;

        SIZE size_nomutex(HASH32 channel_in) const;

        OpRes broadcast(msg_ptr& message, HASH32 channel_out = 0);
        OpRes broadcast_deep(msg_ptr& message, HASH32 channel_out = 0);

       public:
        void setRecipient(Recipient* recipient, HASH32 channel_out = 0, HASH32 channel_in = 0);
        void removeRecipient(Recipient* recipient, HASH32 channel_out = 0);
        // Consume only on success (OR_OK). On OR_Failure the message stays intact.
        OpRes send(msg_ptr& message, HASH32 channel_in = 0);

        // Deep-copy only if it can be accepted.
        OpRes send_deep(const msg_ptr& message, HASH32 channel_in = 0);

        // Create and send a message by id with no fields
        OpRes signal(HASH32 msgid, HASH32 channel_in = 0);

        SIZE size() const;
        SIZE size(HASH32 channel_in) const;

        bool hasMessage() const;
        bool hasMessage(HASH32 channel_in) const;

        SIZE getQueueBytesCount() const;
        SIZE getQueueCount() const;

        SIZE getQueueBytesCount(HASH32 channel_in) const;
        SIZE getQueueCount(HASH32 channel_in) const;

        SIZE getPeakBytes() const;
        SIZE getPeakCount() const;

        SIZE getDroppedCount() const;
        SIZE getRejectedCount() const;

        SIZE getDroppedCount(HASH32 channel_in) const;
        SIZE getRejectedCount(HASH32 channel_in) const;
    };
}  // namespace crb

#endif  // CERBERUS_RECIPIENT_H
