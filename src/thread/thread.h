#ifndef THREAD_THREAD_H
#define THREAD_THREAD_H

/*  This class provides an implementation of a thread.
 *
 *  To run code in using this Thread class, the developer has to write a derived class which extends this one
 *  and overrides the tick() method [protected virtual].
 *
 *  The Thread object can always be started or stopped anytime by any other thread using start() and stop() methods.
 *  When a thread is stopped, it is putted in a paused state; the system scheduler is informed and manages the machines cycles accordingly.
 *  When a thread is started, it simply resumes its loop of code.
 *
 *  To finally terminate the Thread execution, use the terminate() method.
 *
 *  A terminated Thread cannot be resumed.
 */

#include <thread>
#include <chrono>
#include "../Cerberus_global.h"
#include "../mutex/mutex.h"
#include "../time/time.h"

namespace cerberus
{
    namespace thread
    {
        class CERBERUS_EXPORT Thread
        {
            private:
                std::thread m_thread;

                std::chrono::microseconds m_period;

                static void _staticThread(Thread* context);

                void _thread();

                mutex::Mutex m_mutex;

                bool m_periodic;

                bool m_executeFlag; //Do not use directly!

                bool m_terminateFlag; //Do not use directly!

                void _setExecuteFlag(bool state);

                void _setTerminateFlag(bool state);

                bool _getExecuteFlag();

                bool _getTerminateFlag();

                int m_retValue;

            protected:
                virtual int tick();

            public:
                //Constructs a non-periodic thread by default. Passing a valid Time will construct a periodic one.
                Thread(const time::Time& period = time::Time());

                Thread(const Thread& other) = delete;

                //Terminates the Thread if not already terminated, before destructing it
                virtual ~Thread();

                //Starts the thread execution
                void start();

                //Stops the thread execution
                void stop();

                //Blocks until thread terminates and returns the last tick() exit value
                int join();

                //Terminates the Thread
                void terminate();
        };
    }
}

#endif // CERBERUS_H
