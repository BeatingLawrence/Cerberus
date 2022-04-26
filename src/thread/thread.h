#ifndef THREAD_THREAD_H
#define THREAD_THREAD_H

/*  This class provides an implementation of a thread.
 *
 *  To run code using this Thread class, the developer has to write a derived class which extends this one
 *  and overrides the tick() method [protected virtual] (and eventually warmUp() and coolDown()).
 *
 *  The Thread may be of three different types:
 *
 *      - Non-Periodic: The Thread will only wake up from sleep state when a message is pushed into its message queue.
 *                      The start() method will enable the thread and make it consuming all the queue constantly with no delay between cycles.
 *                      The stop() method will disable it. Be careful of this, the message queue could grew up hugely.
 *
 *      - Periodic:     The Thread will wake up from sleep state every time a period of time passes.
 *                      The start() method will resume the cycle.
 *                      The stop() method will pause the cycle.
 *
 *      - One-Shot:     The Thread will run only once.
 *                      The start() method will begin the execution.
 *                      The stop() method does nothing.
 *                      The terminate() method does nothing.
 *
 *  join() can be used in any case to wait for the thread to terminate and to retrieve the return value.
 *
 *  When a periodic Thread is paused or when a Non-Periodic Thread is waiting for messages, the system scheduler is informed
 *  and the Thread will not consume much machine cycles.
 *
 *  A terminated Thread cannot be resumed.
 *
 *  User can access the queue at any time inside the tick() using the nextMessage(); nextMessageKeep() or isQueueEmpty(); methods
 *
 *  warmUp() will be called on the first start, before the first tick() execution.
 *  coolDown() will be called after the last run of tick(), after terminate() is called.
 */

#include <thread>
#include <chrono>
#include "../Cerberus_global.h"
#include "../mutex/mutex.h"
#include "../time/time.h"
#include "./threadbase.h"

namespace cerberus
{
    namespace thread
    {
        class CERBERUS_EXPORT Thread : public cerberus::thread::ThreadBase
        {
            public:
                enum ThreadPeriodicity
                {
                    TP_NonPeriodic,
                    TP_Periodic,
                    TP_OneShot,
                };

            private:
                std::thread m_thread;

                std::chrono::microseconds m_period;

                static void _staticThread(Thread* context);

                void _thread();

                ThreadPeriodicity m_periodicity;

                int m_retValue;

            protected:
                virtual int tick();

                virtual void warmUp();

                virtual void coolDown();

                void sleep(const time::Time& time);

            public:
                //Constructs a non-periodic thread by default. If periodicity is TP_Periodic a valid time must be specified.
                Thread(ThreadPeriodicity periodicity = TP_NonPeriodic, const time::Time& period = time::Time());

                Thread(const Thread& other) = delete;

                Thread(Thread&& other) = delete;

                //Terminates the Thread if not already terminated, before destructing it. Could block (join)
                virtual ~Thread();

                //Starts the thread execution
                void start();

                //Stops the thread execution
                void stop();

                //Blocks until thread terminates and returns the last tick() exit value.
                //If stop is true, the Thread is also terminated.
                int join(bool stop = true);

                //Terminates the Thread
                void terminate();
        };
    }
}

#endif // THREAD_THREAD_H
