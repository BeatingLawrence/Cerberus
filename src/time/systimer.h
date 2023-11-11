#ifndef CERBERUS_TIME_SYSTIMER_H
#define CERBERUS_TIME_SYSTIMER_H

#ifdef LINUX_SYSTEM

#include <signal.h>
#include <time.h>

#include <atomic>

#include "src/time/time.h"
#include "src/types.h"

namespace cerberus
{
    namespace time
    {
        class SysTimer
        {
           private:
            timerCallback m_callback;

            static void defaultTimeoutCallback();

            static void mainCallback(sigval val);

            std::atomic_bool m_running;

            timer_t m_timerId;

            bool m_failed, m_periodic;

            Time m_time;

           public:
            SysTimer();

            SysTimer(const Time& time, bool periodic = false);

            SysTimer(const SysTimer& other) = delete;

            ~SysTimer();

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

#endif
#endif  // CERBERUS_TIME_SYSTIMER_H
