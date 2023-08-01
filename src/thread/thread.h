#ifndef THREAD_THREAD_H
#define THREAD_THREAD_H

/*  This class provides an implementation of a thread.
 *
 *  [Inheritance] To run code using this Thread class, the developer has to write a derived class which extends this one
 *  and overrides the tick() method [protected virtual] (and eventually warmUp() and coolDown()).
 *
 *  [Composition] To run code using this Thread class, the developer can use the provideTickCallback(), provideWarmUpCallback() and
 *  provideCoolDownCallback() to pass some code to the Thread, without the need to write a derived class.
 *  In this case, a message will be always passed to the tick callback and it will be invalid if no message is present in the queue.
 *
 *  The Thread type may be one of these:
 *
 *      - Non-Periodic:  The Thread will constantly wake up as long as messages are present in the queue.
 *                       The start() method will enable the thread and make it consuming all the queue constantly with no delay between cycles.
 *                       The stop() method will disable it. Be careful with this, the message queue could grew up hugely.
 *                       The terminate() method terminates the Thread.
 *
 *      - Periodic:      The Thread will wake up from sleep state every time a period of time passes.
 *                       The start() method resumes the thread.
 *                       The stop() method pauses the thread.
 *                       The terminate() method terminates the Thread.
 *
 *      - PeriodicQueue: The Thread will wake up from sleep state every time a period of time passes.
 *                       If messages are present in the queue, the Thread will temporary act as a Non-Periodic Thread.
 *                       The start() method resumes the thread.
 *                       The stop() method pauses the thread.
 *                       The terminate() method terminates the Thread.
 *
 *      - One-Shot:      The Thread will run only once. It will be terminated at the end of the passed code execution.
 *                       The start() method starts the execution.
 *                       The stop() method does nothing.
 *                       The terminate() method does nothing.
 *
 *  join() can be used for any Thread type to wait for the Thread to terminate and to retrieve the return value.
 *
 *  When a periodic Thread is paused or when a Non-Periodic Thread is waiting for messages, the system scheduler is informed
 *  and the Thread will be de-scheduled.
 *
 *  A terminated Thread cannot be resumed.
 *
 *  User can access the queue at any time inside the tick() using the nextMessage() nextMessageKeep() or isQueueEmpty() methods
 *
 *  warmUp() will be called on the first start, before the first tick() execution.
 *  coolDown() will be called after the last run of tick(), when terminate() is called. When coolDown() execution finishes, the join() returns.
 */

#include "../time/time.h"
#include "./threadbase.h"
#include "src/core/cerberusobject.h"

namespace cerberus
{
    namespace thread
    {
        class CERBERUS_EXPORT Thread : public cerberus::thread::ThreadBase, public CerberusObject
        {
           public:
            enum ThreadPeriodicity
            {
                TP_NonPeriodic,
                TP_Periodic,
                TP_PeriodicQueue,
                TP_OneShot,
            };

           private:
            pthread_t m_pthread;

            time::SplittedTime m_time;

            static void* _staticThread(void* context);

            void _thread();

            ThreadPeriodicity m_periodicity;

            int m_retValue;

            typedef int (*customTickCallback)(message::cerberus_message, Thread*);

            typedef void (*customCallback)();

            customTickCallback m_tickCallback;

            customCallback m_warmUpCallback;

            customCallback m_coolDownCallback;

            static int defaultTickCallback(message::cerberus_message msg, Thread* thread);

            static void defaultWarmUpCallback();

            static void defaultCoolDownCallback();

            void wait();

           protected:
            virtual int tick();

            virtual void warmUp();

            virtual void coolDown();

           public:
            // Construct a non-periodic thread by default. If periodicity is TP_Periodic a valid time must be specified.
            Thread(const std::string& name, ThreadPeriodicity periodicity = TP_NonPeriodic, const time::Time& time = time::Time());

            Thread(const Thread& other) = delete;

            Thread(Thread&& other) = delete;

            // Terminate the Thread if not already terminated, before destructing it. Could block (join)
            virtual ~Thread();

            // Put the calling thread in sleep state for a given time
            static void sleep(const time::Time& time);

            // Start the thread execution
            void start();

            // Stop the thread execution
            void stop();

            // Block until thread terminates and return the last tick() exit value.
            // If stop is true, the Thread is also terminated.
            int join(bool stop = false);

            // Detach the Thread from the owner Thread
            void detach();

            // Terminate the Thread. This operation is irreversible
            void terminate();

            // Set a custom callback to be executed as tick()
            void provideTickCallback(customTickCallback callback);

            // Set a custom callback to be executed as warmUp()
            void provideWarmUpCallback(customCallback callback);

            // Set a custom callback to be executed as coolDown()
            void provideCoolDownCallback(customCallback callback);
        };
    }  // namespace thread
}  // namespace cerberus

#endif  // THREAD_THREAD_H
