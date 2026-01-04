#ifndef CERBERUS_RECIPIENT_H
#define CERBERUS_RECIPIENT_H

#include <list>

#include "../Cerberus_global.h"
#include "../types.h"
#include "./mutex.h"

namespace cerberus
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
            SIZE index{0};
            std::list<msg_ptr> queue;

            SIZE queueBytes{0};
            SIZE queueLimitBytes{0};  // 0 = unlimited
            SIZE queueLimitCount{0};  // 0 = unlimited

            SIZE peakBytes{0};
            SIZE peakCount{0};

            SIZE dropped{0};   // only used by DROP_OLDEST
            SIZE rejected{0};  // send()/send_deep() refused due to limits

            OverflowPolicy policy{REJECT_NEW};
        };

        std::list<QueueState> m_queues;

        // Total peaks across all queues
        SIZE m_peakBytesTotal;
        SIZE m_peakCountTotal;

        mutable Mutex* m_mutex;
        bool m_extmutex;

        void _updatePeaks_nomutex(QueueState& q);
        void _updateTotalPeaks_nomutex();
        bool _overLimit_nomutex(const QueueState& q, SIZE addBytes, SIZE addCount) const;
        void _dropOldest_nomutex(QueueState& q, SIZE bytesToFit, SIZE countToFit);

        QueueState* _q_nomutex(SIZE queueIndex);
        const QueueState* _q_nomutex(SIZE queueIndex) const;
        QueueState* _ensureQueue_nomutex(SIZE queueIndex);

        SIZE _totalBytes_nomutex() const;
        SIZE _totalCount_nomutex() const;

       protected:
        Recipient(Mutex* mutex = nullptr);
        virtual ~Recipient();

        msg_ptr next();
        msg_ptr nextKeep() const;

        msg_ptr next(SIZE queueIndex);
        msg_ptr nextKeep(SIZE queueIndex) const;

        void clear();

        void clear(SIZE queueIndex);

        void setQueueLimitBytes(SIZE bytes);  // 0 = unlimited
        void setQueueLimitCount(SIZE count);  // 0 = unlimited
        void setOverflowPolicy(OverflowPolicy p);

        void setQueueLimitBytes(SIZE bytes, SIZE queueIndex);
        void setQueueLimitCount(SIZE count, SIZE queueIndex);
        void setOverflowPolicy(OverflowPolicy p, SIZE queueIndex);

        void resetStats();

        void resetStats(SIZE queueIndex);

        // Re-insert a message bypassing limits
        OpRes requeueFront(msg_ptr& message);  // consumes only on OR_OK
        OpRes requeueBack(msg_ptr& message);   // consumes only on OR_OK

        OpRes requeueFront(msg_ptr& message, SIZE queueIndex);
        OpRes requeueBack(msg_ptr& message, SIZE queueIndex);

        virtual void newMsg();
        virtual void newMsg_first();

        SIZE size_nomutex() const;

        SIZE size_nomutex(SIZE queueIndex) const;

       public:
        // Consume only on success (OR_OK). On OR_Failure the message stays intact.
        OpRes send(msg_ptr& message);

        // Same as send(), but targets a specific internal queue.
        OpRes send(msg_ptr& message, SIZE queueIndex);

        // Deep-copy only if it can be accepted.
        OpRes send_deep(const msg_ptr& message);

        // Same as send_deep(), but targets a specific internal queue.
        OpRes send_deep(const msg_ptr& message, SIZE queueIndex);

        SIZE size() const;
        SIZE size(SIZE queueIndex) const;

        bool hasMessage() const;
        bool hasMessage(SIZE queueIndex) const;

        SIZE getQueueBytesCount() const;
        SIZE getQueueCount() const;

        SIZE getQueueBytesCount(SIZE queueIndex) const;
        SIZE getQueueCount(SIZE queueIndex) const;

        SIZE getPeakBytes() const;
        SIZE getPeakCount() const;

        SIZE getDroppedCount() const;
        SIZE getRejectedCount() const;

        SIZE getDroppedCount(SIZE queueIndex) const;
        SIZE getRejectedCount(SIZE queueIndex) const;
    };
}  // namespace cerberus

#endif  // CERBERUS_RECIPIENT_H
