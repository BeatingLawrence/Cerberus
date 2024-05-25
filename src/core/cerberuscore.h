#ifndef CERBERUS_CORE_CERBERUSCORE_H
#define CERBERUS_CORE_CERBERUSCORE_H

/*  This is the CerberusCore class.
 *
 *  The CerberusCore is the main thread of execution of the framework.
 *  It's used to route messages, log on file, create other Threads and other stuff..
 *
 */

#include "../thread/thread.h"
#include "../thread/threadpool.h"
#include "eventscheduler.h"

namespace cerberus
{
    class Timer;

    namespace core
    {
        class CerberusCore : public cerberus::Thread
        {
           private:
            CoreConf m_conf;

            ThreadPool m_pool;

            void initializeThreadPool();
            void deinitializeThreadPool();

            virtual int tick() override;
            virtual void warmUp() override;
            virtual void coolDown() override;

            void processTaskMsg(cerberus_message msg);
            void processMsg(cerberus_message msg);

           public:
            EventScheduler m_eventScheduler;

            CerberusCore();

            virtual ~CerberusCore();

            void setup(const CoreConf& parms);
        };
    }  // namespace core
}  // namespace cerberus

#endif  // CERBERUS_CORE_CERBERUSCORE_H
