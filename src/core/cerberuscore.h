#ifndef CERBERUS_CORE_CERBERUSCORE_H
#define CERBERUS_CORE_CERBERUSCORE_H

/*  This is the CerberusCore class.
 *
 *  The CerberusCore is the main thread of execution of the framework.
 *  It's used to route messages, log on file, create other Threads and other stuff..
 *
 */

#include "../thread/thread.h"
#include "eventscheduler.h"

namespace cerberus
{
    namespace core
    {
        class CerberusCore : public cerberus::thread::Thread
        {
           private:
            virtual int tick() override;

            virtual void warmUp() override;

            virtual void coolDown() override;

           public:
            EventScheduler m_eventScheduler;

            CerberusCore();
        };
    }  // namespace core
}  // namespace cerberus

#endif  // CERBERUS_CORE_CERBERUSCORE_H
