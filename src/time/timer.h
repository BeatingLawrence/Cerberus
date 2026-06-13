#ifndef CERBERUS_TIME_TIMER_H
#define CERBERUS_TIME_TIMER_H

#include <atomic>

#include "../types.h"
#include "timeframe.h"

namespace crb
{
    class Timer
    {
       private:
       std::atomic_bool m_running;
        std::atomic_bool m_expired;

        TimerType m_type;

        TimeFrame m_time;

        DateTime m_delay;

        timerCallback m_callback;
        void* m_ctx;
        HASH32 m_recipient;

        static void defaultTimeoutCallback(void* ctx);

       public:
        CERBERUS_EXPORT Timer();

        CERBERUS_EXPORT Timer(const TimeFrame& time, TimerType type = TT_OneShot,
                              const DateTime& delay = DateTime());

        Timer(const Timer& other) = delete;

        CERBERUS_EXPORT ~Timer();

        CERBERUS_EXPORT void setTime(const TimeFrame& time, const DateTime& delay = DateTime());

        CERBERUS_EXPORT void setTime(const DateTime& delay);

        CERBERUS_EXPORT void start();

        CERBERUS_EXPORT void stop();

        CERBERUS_EXPORT bool isRunning();
        CERBERUS_EXPORT bool expired();

        // Sets a callback to be executed when the timer expires
        CERBERUS_EXPORT void provideTimeoutCallback(timerCallback callback, void* ctx);

        // Sets a recipient to be notified when the timer expires
        CERBERUS_EXPORT void setRecipient(HASH32 recipientId);
    };
}  // namespace crb

#endif  // CERBERUS_TIME_TIMER_H
