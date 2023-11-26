#ifndef CERBERUS_TIME_TIMER_H
#define CERBERUS_TIME_TIMER_H

#include <atomic>

#include "src/time/timeframe.h"
#include "src/types.h"

namespace cerberus
{
    namespace time
    {
        class Timer
        {
           private:
            std::atomic_bool m_running;

            bool m_periodic;

            TimeFrame m_time;

            timerCallback m_callback;

            static void defaultTimeoutCallback();

           public:
            Timer();

            Timer(const TimeFrame& time, bool periodic = false);

            Timer(const Timer& other) = delete;

            ~Timer();

            void setTime(const TimeFrame& time);

            void start();

            void stop();

            void reset();

            bool isRunning();

            // Sets a callback to be executed when the timer expires
            void provideTimeoutCallback(timerCallback callback);
        };
    }  // namespace time
}  // namespace cerberus

#endif  // CERBERUS_TIME_TIMER_H
