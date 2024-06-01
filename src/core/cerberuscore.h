#ifndef CERBERUS_CORE_CERBERUSCORE_H
#define CERBERUS_CORE_CERBERUSCORE_H

/*  This is the CerberusCore class.
 *
 *  The CerberusCore is the main thread of execution of the framework.
 *  It's used to route messages, manage thread pool, sockets and other stuff..
 *
 */

#include "../thread/thread.h"
#include "../thread/threadpool.h"
#include "eventscheduler.h"
#include "sockmanager.h"

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

            SockManager m_socks;

            void initializeThreadPool();
            void deinitializeThreadPool();

            virtual int tick() override;
            virtual void warmUp() override;
            virtual void coolDown() override;

            void processTaskMsg(cerberus_message msg);
            void processMsg(cerberus_message msg);

            static OpRes sockCB(void* ctx, void* data);

            static void processClient(CerberusCore* ctx, Socket* sock, SockManager::SockData* parentData,
                                      const SockSettings& settings);

           public:
            EventScheduler m_eventScheduler;

            CerberusCore();

            virtual ~CerberusCore();

            void setup(const CoreConf& parms);

            //=====================SOCKETS========================

            // Create a new socket in the Cerberus memory space
            OpResData<CHANDLE> newSock(const SockSettings& settings);

            // Add a listener to the specified socket
            OpRes addSockListener(CHANDLE sock, HASH32 threadID);

            // Send out a buffer using the specified managed socket
            OpRes sockSend(CHANDLE sock, const ByteBuffer& buffer);

            // Remove the specified socket
            OpRes removeSock(CHANDLE sock);
        };
    }  // namespace core
}  // namespace cerberus

#endif  // CERBERUS_CORE_CERBERUSCORE_H
