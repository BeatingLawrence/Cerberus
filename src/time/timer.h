#ifndef CERBERUS_TIME_TIMER_H
#define CERBERUS_TIME_TIMER_H

#include <atomic>

#include "../types.h"
#include "timeframe.h"

namespace cerberus
{
    class Timer
    {
       private:
        std::atomic_bool m_running;

        bool m_periodic;

        TimeFrame m_time;

        timerCallback m_callback;
        void* m_ctx;

        static void defaultTimeoutCallback(void* ctx);

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
        void provideTimeoutCallback(timerCallback callback, void* ctx);
    };
}  // namespace cerberus

#endif  // CERBERUS_TIME_TIMER_H
