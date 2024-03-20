#ifndef CERBERUS_CORE_EVENTSCHEDULER_H
#define CERBERUS_CORE_EVENTSCHEDULER_H

#include <atomic>
#include <vector>

#include "../mutex/mutex.h"
#include "../thread/thread.h"
#include "../time/datetime.h"

namespace cerberus
{
    namespace core
    {
        class EventScheduler : public cerberus::Thread
        {
           private:
            struct TimerData
            {
                std::atomic_bool* bit;
                DateTime expiryDate;  // point in time when the timer will fire
                TimeFrame time;       // amount of time between fires (if valid, timer is periodic)
                timerCallback callback;
            };

            std::vector<TimerData> m_timers;

            Mutex m_mutex;

            virtual int tick() override;

            void addTimer(std::atomic_bool& bit, DateTime d, TimeFrame t, timerCallback callback);

           public:
            EventScheduler();

            virtual ~EventScheduler();

            // Start a new periodic timer that will fire every t time
            void startTimer(std::atomic_bool& bit, TimeFrame t, timerCallback callback);

            // Start a new periodic timer that will fire at d (the first time) and then, every t time
            void startTimer(std::atomic_bool& bit, DateTime d, TimeFrame t, timerCallback callback);

            // Start a new one-shot timer that will fire at d and then it will be removed from the references
            // as stopTimer() were called
            void startTimer(std::atomic_bool& bit, DateTime d, timerCallback callback);

            // Stop a timer and remove it from references
            void stopTimer(std::atomic_bool& bit);
        };
    }  // namespace core
}  // namespace cerberus

#endif  // CERBERUS_CORE_EVENTSCHEDULER_H
