#ifndef CERBERUS_CORE_FASTLOOP_H
#define CERBERUS_CORE_FASTLOOP_H

#include <atomic>
#include <vector>

#include "src/mutex/mutex.h"
#include "src/thread/thread.h"

namespace cerberus
{
    namespace core
    {
        class FastLoop : public cerberus::thread::Thread
        {
           private:
            struct TimerData
            {
                std::atomic_bool* bit;
                uint32_t time;    // current counting time expressed in hundreds of us
                uint32_t target;  // target firing time expressed in hundreds of us
                bool periodic;
                timerCallback callback;
            };

            std::vector<TimerData> m_timers;

            mutex::Mutex m_mutex;

            virtual int tick() override;

           public:
            FastLoop();

            virtual ~FastLoop();

            void startTimer(std::atomic_bool& bit, uint32_t target, bool periodic, timerCallback callback);

            void stopTimer(std::atomic_bool& bit);
        };

    }  // namespace core
}  // namespace cerberus

#endif  // CERBERUS_CORE_FASTLOOP_H
