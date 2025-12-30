#ifndef RECIPIENT_H
#define RECIPIENT_H

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
        std::list<msg_ptr> m_queue;

        SIZE m_queueBytes;
        SIZE m_queueWarningBytes;
        SIZE m_queueLimitBytes;

        SIZE m_queueWarningCount;
        SIZE m_queueLimitCount;

        SIZE m_peakBytes;
        SIZE m_peakCount;

        SIZE m_dropped;
        SIZE m_rejected;

        bool m_warnedBytes;
        bool m_warnedCount;

        OverflowPolicy m_policy;

        mutable Mutex* m_mutex;
        bool m_extmutex;

        void _updatePeaks_nomutex();
        void _checkWarnings_nomutex();
        bool _overLimit_nomutex(SIZE addBytes, SIZE addCount) const;
        void _dropOldest_nomutex(SIZE bytesToFit, SIZE countToFit);

       protected:
        Recipient(Mutex* mutex = nullptr);
        virtual ~Recipient();

        msg_ptr next();
        msg_ptr nextKeep() const;

        void clear();

        void setQueueWarning(SIZE bytes);

        void setQueueWarningBytes(SIZE bytes);
        void setQueueWarningCount(SIZE count);

        void setQueueLimitBytes(SIZE bytes);  // 0 = unlimited
        void setQueueLimitCount(SIZE count);  // 0 = unlimited
        void setOverflowPolicy(OverflowPolicy p);

        void resetStats();

        virtual void newMsg();
        virtual void newMsg_first();

        SIZE size_nomutex() const;

       public:
        OpRes send(msg_ptr& message);  // consume only on success

        OpRes send_deep(const msg_ptr& message, HASH32 recipient = CERBERUS_INVALID_ID);

        SIZE size() const;
        bool hasMessage() const;

        SIZE getQueueBytesCount() const;
        SIZE getQueueCount() const;

        SIZE getPeakBytes() const;
        SIZE getPeakCount() const;

        SIZE getDroppedCount() const;
        SIZE getRejectedCount() const;
    };
}  // namespace cerberus

#endif  // RECIPIENT_H
