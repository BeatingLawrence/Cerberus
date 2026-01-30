#ifndef CERBERUS_CORE_EVENTSCHEDULER_H
#define CERBERUS_CORE_EVENTSCHEDULER_H

#include <atomic>
#include <vector>

#include "src/thread/mutex.h"
#include "src/thread/thread.h"
#include "src/time/datetime.h"
#include "src/time/timeframe.h"
#include "src/types.h"

namespace crb::core
{
    class EventScheduler : public crb::Thread
    {
       private:
        Mutex m_mutex;
        std::vector<TimerData> m_timers;
        msg_ptr m_timerExpiryMsg;

        void addTimer(std::atomic_bool* bit, std::atomic_bool* expired, DateTime d, TimeFrame t,
                      timerCallback callback, void* ctx, HASH32 recipient);

        int tick() override;

       public:
        EventScheduler();
        ~EventScheduler() override;

        void startTimer(TimerData& data);
        void stopTimer(std::atomic_bool& bit);
    };
}  // namespace crb::core

#endif  // CERBERUS_CORE_EVENTSCHEDULER_H
