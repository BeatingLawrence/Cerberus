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
        std::atomic_bool m_expired;

        TimerType m_type;

        TimeFrame m_time;

        DateTime m_delay;

        timerCallback m_callback;
        void* m_ctx;
        HASH32 m_recipient;

        static void defaultTimeoutCallback(void* ctx);

       public:
        Timer();

        Timer(const TimeFrame& time, TimerType type = TT_OneShot, const DateTime& delay = DateTime());

        Timer(const Timer& other) = delete;

        ~Timer();

        void setTime(const TimeFrame& time, const DateTime& delay = DateTime());

        void setTime(const DateTime& delay);

        void start();

        void stop();

        bool isRunning();
        bool expired();

        // Sets a callback to be executed when the timer expires
        void provideTimeoutCallback(timerCallback callback, void* ctx);

        // Sets a recipient to be notified when the timer expires
        void setRecipient(HASH32 recipientId);
    };
}  // namespace cerberus

#endif  // CERBERUS_TIME_TIMER_H
