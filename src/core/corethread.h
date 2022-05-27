#ifndef CERBERUS_CORE_CORETHREAD_H
#define CERBERUS_CORE_CORETHREAD_H

#include <thread>
#include <chrono>
#include "../thread/threadbase.h"

namespace cerberus
{
    namespace core
    {
        class CoreThread : public cerberus::thread::ThreadBase
        {
            private:
                std::thread m_thread;

                static void _staticThread(CoreThread* context);

                void _thread();

                int m_retValue;

            protected:
                virtual int tick() = 0;

                virtual void warmUp() = 0;

                virtual void coolDown() = 0;

                //Constructs a non-periodic Core Thread
                CoreThread();

                CoreThread(const CoreThread& other) = delete;

                CoreThread(CoreThread&& other) = delete;

                //Terminates the Thread if not already terminated, before destructing it. Could block (join)
                ~CoreThread();

            public:
                //Starts the thread execution
                void start();

                //Stops the thread execution
                void stop();

                //Blocks until thread terminates and returns the last tick() exit value.
                //If stop is true (default), the Thread is also terminated.
                int join(bool stop = true);

                //Terminates the Thread. This operation is irreversible
                void terminate();
        };
    }
}

#endif // THREAD_THREAD_H
