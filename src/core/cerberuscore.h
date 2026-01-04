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
#include "socketmanager.h"
#include "cerberusregister.h"

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

            SocketManager m_sockets;

            CerberusRegister m_reg;

            void initializeThreadPool();
            void deinitializeThreadPool();

            virtual int tick() override;
            virtual void warmUp() override;
            virtual void coolDown() override;

            void processTaskMsg(msg_ptr& msg);
            void processMsg(msg_ptr& msg);

            static OpRes socketCB(void* ctx, void* data);

            static void processClient(CerberusCore* ctx, cerberus_socket socket,
                                      SocketManager::SocketData* parentData, const SocketSettings& settings);

           public:
            EventScheduler m_eventScheduler;

            CerberusCore();

            virtual ~CerberusCore();

            void setup(const CoreConf& parms);

            //=====================REGISTER========================

            void registerObj(Recordable* object) { m_reg.registerObj(object); }
            void unregisterObj(HASH32 id) { m_reg.unregisterObj(id); }
            HASH32 objIdByName(const std::string& name) { return m_reg.objIdByName(name); }

            OpRes sendMsgToObj(HASH32 id, msg_ptr& msg) { return m_reg.sendMsgToObj(id, msg); }
            OpRes sendMsgToObj(const std::string& name, msg_ptr& msg) { return m_reg.sendMsgToObj(name, msg); }
            OpRes sendMsgToObj(HASH32 id, msg_ptr& msg, SIZE queueIndex) { return m_reg.sendMsgToObj(id, msg, queueIndex); }
            OpRes sendMsgToObj(const std::string& name, msg_ptr& msg, SIZE queueIndex)
            {
                return m_reg.sendMsgToObj(name, msg, queueIndex);
            }

            OpRes sendMsgToObj_deep(HASH32 id, const msg_ptr& msg) { return m_reg.sendMsgToObj_deep(id, msg); }
            OpRes sendMsgToObj_deep(const std::string& name, const msg_ptr& msg) { return m_reg.sendMsgToObj_deep(name, msg); }
            OpRes sendMsgToObj_deep(HASH32 id, const msg_ptr& msg, SIZE queueIndex)
            {
                return m_reg.sendMsgToObj_deep(id, msg, queueIndex);
            }
            OpRes sendMsgToObj_deep(const std::string& name, const msg_ptr& msg, SIZE queueIndex)
            {
                return m_reg.sendMsgToObj_deep(name, msg, queueIndex);
            }


            OpRes addMsgTemplate(const msg_ptr& tmplt) { return m_reg.addMsgTemplate(tmplt); }
            msg_ptr constructMessage(HASH32 id) { return m_reg.constructMessage(id); }

            HASH32 addPlugin(void* handle, const std::string& path, bool& exists) { return m_reg.addPlugin(handle, path, exists); }
            MutexLocker getPluginMutex(HASH32 id) { return m_reg.getPluginMutex(id); }
            void* checkPlugin(HASH32 id) { return m_reg.checkPlugin(id); }
            bool updatePlugin(HASH32 id, const std::string& path, void* handle) { return m_reg.updatePlugin(id, path, handle); }
            void cleanupPlugins() { m_reg.cleanupPlugins(); }

            //=====================SOCKETS========================

            // Create a new socket in the Cerberus memory space
            OpResData<CHANDLE> newSocket(const SocketSettings& settings);

            // Add a listener to the specified socket
            OpRes addSocketListener(CHANDLE socket, HASH32 threadID);

            // Send out a buffer using the specified managed socket
            OpRes socketSend(CHANDLE socket, const ByteBuffer& buffer);

            // Remove the specified socket
            OpRes removeSocket(CHANDLE socket);
        };
    }  // namespace core
}  // namespace cerberus

#endif  // CERBERUS_CORE_CERBERUSCORE_H
