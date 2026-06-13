#ifndef CERBERUS_TIME_SYSTIMER_H
#define CERBERUS_TIME_SYSTIMER_H

#include <atomic>
#include <chrono>

#ifndef WINDOWS_SYSTEM
#include <signal.h>
#include <time.h>
#endif

#include "../types.h"
#include "./timeframe.h"

namespace crb
{
    namespace time
    {
        class SysTimer
        {
           private:
            timerCallback m_callback;
            void* m_ctx;

            static void defaultTimeoutCallback(void* ctx);

#ifndef WINDOWS_SYSTEM
            static void mainCallback(sigval val);
#endif

            bool ensureTimer();

            std::atomic_bool m_running;

#ifndef WINDOWS_SYSTEM
            timer_t m_timerId;
#endif

            bool m_timerCreated;

            bool m_failed, m_periodic;

            TimeFrame m_time;

            uint64_t m_periodNs;
#ifdef WINDOWS_SYSTEM
            std::chrono::steady_clock::time_point m_nextDeadline;
#else
            timespec m_nextDeadline;
#endif
            bool m_deadlineArmed;
            bool m_overrun;

           public:
            SysTimer();

            SysTimer(const TimeFrame& time, bool periodic = false);

            SysTimer(const SysTimer& other) = delete;

            ~SysTimer();

            // Set the relative timer period used by callback and deadline modes.
            void setTime(const TimeFrame& time);

            //
            // Callback timer API.
            //

            // Arm the callback timer from now using the configured period.
            void start();

            // Disarm the callback timer.
            void stop();

            // Restart the callback timer from now.
            void reset();

            // Return true while the callback timer is armed.
            bool isRunning();

            // Set the callback executed when the callback timer expires.
            void provideTimeoutCallback(timerCallback callback, void* ctx = nullptr);

            //
            // Deadline timer API.
            //

            // Arm the next absolute deadline at now + period.
            void startDeadline();

            // Sleep until the current deadline and return true if it was missed.
            bool waitDeadline();

            // Return true if the last timer operation failed.
            bool isFailed();

            // Return true if the last deadline wait detected a missed period.
            bool isOverrun() const;
        };
    }  // namespace time
}  // namespace crb

#endif  // CERBERUS_TIME_SYSTIMER_H
