#ifndef CERBERUS_CORE_CERBERUSCORE_H
#define CERBERUS_CORE_CERBERUSCORE_H

/*  This is the CerberusCore class.
 *
 *  The CerberusCore is the main thread of execution of the framework.
 *  It's used to route messages, log on file, create other Threads and other stuff..
 *
 */

#include "../thread/player.h"
#include "../thread/thread.h"
#include "eventscheduler.h"

namespace cerberus
{
    namespace core
    {
        class CerberusCore : public cerberus::Thread
        {
           private:
            CoreConf m_conf;

            std::list<Player*> m_pool;          // manual triggered
            std::list<Player*> m_reservedPool;  // message triggered

            void initializeThreadPool();
            void deinitializeThreadPool();

            void cleanupPlayer(Thread* t);

            static void taskEndCb(void* ctx, Player* thread, OpRes res);
            void _taskEndCb(Player* thread, OpRes res);

            virtual int tick() override;

            virtual void warmUp() override;

            virtual void coolDown() override;

            void processTaskMsg(cerberus_message msg);
            void processMsg(cerberus_message msg);

            void runTask(Task t);

           public:
            EventScheduler m_eventScheduler;

            CerberusCore();

            virtual ~CerberusCore();

            void setup(const CoreConf& parms);
        };
    }  // namespace core
}  // namespace cerberus

#endif  // CERBERUS_CORE_CERBERUSCORE_H
