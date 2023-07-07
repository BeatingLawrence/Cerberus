#ifndef CERBERUS_TIME_TIMER_H
#define CERBERUS_TIME_TIMER_H

#include <signal.h>
#include <time.h>

#include <atomic>

#include "time.h"

namespace cerberus
{
    namespace time
    {
        class Timer
        {
           private:
            typedef void (*timerCallback)();

            timerCallback m_callback;

            static void defaultTimeoutCallback();

            static void mainCallback(sigval val);

            std::atomic_bool m_running;

            timer_t m_timerId;

            bool m_failed, m_periodic;

            Time m_time;

           public:
            Timer();

            Timer(const Time& time, bool periodic = false);

            Timer(const Timer& other) = delete;

            void setTime(const Time& time);

            void start();

            void stop();

            void reset();

            bool isRunning();

            bool isFailed();

            // Sets a callback to be executed when the timer expires
            void provideTimeoutCallback(timerCallback callback);
        };
    }  // namespace time
}  // namespace cerberus

#endif  // CERBERUS_TIME_TIMER_H
