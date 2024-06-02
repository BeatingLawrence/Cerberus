#ifndef CERBERUS_CORE_EVENTSCHEDULER_H
#define CERBERUS_CORE_EVENTSCHEDULER_H

#include <atomic>
#include <vector>

#include "../thread/mutex.h"
#include "../thread/thread.h"
#include "../time/datetime.h"

namespace cerberus
{
    namespace core
    {
        class EventScheduler : public cerberus::Thread
        {
           private:
            std::vector<TimerData> m_timers;

            Mutex m_mutex;

            virtual int tick() override;

            void addTimer(std::atomic_bool* bit, DateTime d, TimeFrame t, timerCallback callback, void* ctx);

           public:
            EventScheduler();

            virtual ~EventScheduler();

            // Start a timer. The configuration depends on the validity of data members
            void startTimer(TimerData& data);

            // Stop a timer and remove it from references
            void stopTimer(std::atomic_bool& bit);
        };
    }  // namespace core
}  // namespace cerberus

#endif  // CERBERUS_CORE_EVENTSCHEDULER_H
