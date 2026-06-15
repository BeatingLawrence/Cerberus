#ifndef THREAD_THREAD_H
#define THREAD_THREAD_H

/*  This class provides an implementation of a thread.
 *
 *  [Inheritance] To run code using this Thread class, the developer has to write a derived class which
 * extends this one and overrides the tick() method [protected virtual] (and eventually warmUp() and
 * coolDown()).
 *
 *  [Composition] To run code using this Thread class, the developer can use the provideTickCallback(),
 * provideWarmUpCallback() and provideCoolDownCallback() to pass some code to the Thread, without the need to
 * write a derived class. In this case, a message will be always passed to the tick callback and it will be
 * invalid if no message is present in the queue.
 *
 *  The Thread type may be one of these:
 *
 *      - Non-Periodic:  The Thread will constantly wake up as long as messages are present in the queue.
 *                       The start() method will enable the thread and make it consuming all the queue
 * constantly with no delay between cycles. The stop() method will disable it. Be careful with this, the
 * message queue could grew up hugely. The terminate() method terminates the Thread.
 *
 *      - Periodic:      The Thread will wake up from sleep state every time a period of time passes.
 *                       The start() method resumes the thread.
 *                       The stop() method pauses the thread.
 *                       The terminate() method terminates the Thread.
 *
 *      - Periodic RT:   Same as Periodic, but the thread is started with real-time scheduling.
 *
 *      - PeriodicQueue: The Thread will wake up from sleep state every time a period of time passes.
 *                       If messages are present in the queue, the Thread will temporary act as a Non-Periodic
 * Thread. The start() method resumes the thread. The stop() method pauses the thread. The terminate() method
 * terminates the Thread.
 *
 *      - One-Shot:      The Thread will run only once. It will be terminated at the end of the passed code
 * execution. The start() method starts the execution. The stop() method does nothing. The terminate() method
 * does nothing.
 *
 *  join() can be used for any Thread type to wait for the Thread to terminate and to retrieve the return
 * value.
 *
 *  When a periodic Thread is paused or when a Non-Periodic Thread is waiting for messages, the system
 * scheduler is informed and the Thread will be de-scheduled.
 *
 *  A terminated Thread cannot be resumed.
 *
 *  User can access the queue at any time inside the tick() using the nextMessage() nextMessageKeep() or
 * isQueueEmpty() methods
 *
 *  warmUp() will be called on the first start, before the first tick() execution.
 *  coolDown() will be called after the last run of tick(), when terminate() is called. When coolDown()
 * execution finishes, the join() returns.
 */

#include "../time/systimer.h"
#include "../time/timeframe.h"
#include "./threadbase.h"

#if !defined(WINDOWS_SYSTEM) && !defined(_WIN32) && !defined(WIN32)
#include <pthread.h>
#endif

namespace crb
{
    class Thread : public ThreadBase
    {
       private:
#if defined(WINDOWS_SYSTEM) || defined(_WIN32) || defined(WIN32)
        void* m_threadHandle;
        unsigned m_threadId;
#else
        pthread_t m_pthread;
#endif

        SplittedTime m_time;
        time::SysTimer m_periodTimer;
        bool m_overrun;

        void* m_stack;
        LSIZE m_stackSize;

#if defined(WINDOWS_SYSTEM) || defined(_WIN32) || defined(WIN32)
        static unsigned __stdcall _staticThread(void* context);
#else
        static void* _staticThread(void* context);
#endif

        void _thread();

        int m_retValue;

        threadTickCallback m_tickCallback;

        threadCallback m_warmUpCallback, m_coolDownCallback;

        static CoreSet s_defaultCoreSet;

        static int defaultTickCallback(msg_ptr msg, Thread* thread);

        static void defaultWarmUpCallback(Thread* thread);

        static void defaultCoolDownCallback(Thread* thread);

        void _wait();

        void _construct(ThreadPeriodicity periodicity, const TimeFrame& time, LSIZE stackSize,
                        const CoreSet& coreSet);

        std::string m_threadName;

       protected:
        CERBERUS_EXPORT virtual int tick();

        CERBERUS_EXPORT virtual void warmUp();

        CERBERUS_EXPORT virtual void coolDown();

       public:
        // Set thread name if supported by the platform.
        CERBERUS_EXPORT void setThreadName(const std::string& name);
        const std::string& getThreadName() const { return m_threadName; }

        // Construct a thread with optional stack size.
        // If periodicity is TP_Periodic, TP_Periodic_realtime, or TP_PeriodicMessage a valid time must be specified.
        CERBERUS_EXPORT Thread(ThreadPeriodicity periodicity, const TimeFrame& time,
                               LSIZE stackSize = 0, const CoreSet& coreSet = CoreSet());

        // Construct a non-periodic thread with optional stack size
        CERBERUS_EXPORT Thread(LSIZE stackSize = 0, const CoreSet& coreSet = CoreSet());

        // Construct a non-timed thread with given periodicity and optional stack size.
        // TP_Continuos_realtime uses realtime scheduling without a period timer.
        CERBERUS_EXPORT Thread(ThreadPeriodicity periodicity, LSIZE stackSize = 0,
                               const CoreSet& coreSet = CoreSet());

        Thread(const Thread& other) = delete;

        Thread(Thread&& other) = delete;

        CERBERUS_EXPORT virtual ~Thread();

        CERBERUS_EXPORT void checkIn(const std::string& name) override;

        CERBERUS_EXPORT SplittedTime getTime() const;

        CERBERUS_EXPORT bool isOverrun() const;

        // Put the calling thread in sleep state for a given time
        CERBERUS_EXPORT static void sleep(const TimeFrame& time);

        // Block until thread terminates and return the last tick() exit value.
        // If stop is true, the Thread is also started and terminated.
        CERBERUS_EXPORT IntOpRes join(bool stop = false);

        // Detach the Thread from the owner Thread
        CERBERUS_EXPORT OpRes detach();

        // Set a custom callback to be executed as tick()
        CERBERUS_EXPORT void provideTickCallback(threadTickCallback callback);

        // Set a custom callback to be executed as warmUp()
        CERBERUS_EXPORT void provideWarmUpCallback(threadCallback callback);

        // Set a custom callback to be executed as coolDown()
        CERBERUS_EXPORT void provideCoolDownCallback(threadCallback callback);

       private:
        // Set default core set for new threads (empty = no default affinity).
        static void setDefaultCoreSet(const CoreSet& coreSet);

        friend class Cerberus;
    };
}  // namespace crb

#endif  // THREAD_THREAD_H
