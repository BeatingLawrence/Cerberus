#ifndef CERBERUS_CORE_CERBERUSCORE_H
#define CERBERUS_CORE_CERBERUSCORE_H

/*  This is the CerberusCore class.
 *
 *  The CerberusCore is the main thread of execution of the framework.
 *  It's used to route messages, manage thread pool, sockets and other stuff..
 *
 */

#include <utility>

#include "../thread/thread.h"
#include "../thread/threadpool.h"
#include "../data/filesystem/inidatafile.h"
#include "eventscheduler.h"
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

            CerberusRegister m_reg;

            IniDataFile m_iniFile;

            void initializeThreadPool();
            void deinitializeThreadPool();

            virtual int tick() override;
            virtual void warmUp() override;
            virtual void coolDown() override;

            void processTaskMsg(msg_ptr& msg);
            void processMsg(msg_ptr& msg);

           public:
            EventScheduler m_eventScheduler;

            CerberusCore();

            virtual ~CerberusCore();

            void setup(const CoreConf& parms);

            IniDataFile& iniFile() { return m_iniFile; }
            const IniDataFile& iniFile() const { return m_iniFile; }
            void setIniFile(const IniDataFile& other) { m_iniFile = other; }
            void setIniFile(IniDataFile&& other) { m_iniFile = std::move(other); }
            void setIniFileName(const std::string& fileName) { m_iniFile.setFileName(fileName); }

            //=====================REGISTER========================

            void registerObj(Recordable* object, const std::string& name) { m_reg.registerObj(object, name); }
            void unregisterObj(HASH32 id) { m_reg.unregisterObj(id); }
            OpRes sendMsgToObj(HASH32 id, msg_ptr& msg, HASH32 channel_in = 0)
            {
                return m_reg.sendMsgToObj(id, msg, channel_in);
            }

            OpRes sendMsgToObj_deep(HASH32 id, const msg_ptr& msg, HASH32 channel_in = 0)
            {
                return m_reg.sendMsgToObj_deep(id, msg, channel_in);
            }


            OpRes addMsgTemplate(const msg_ptr& tmplt) { return m_reg.addMsgTemplate(tmplt); }
            msg_ptr constructMessage(HASH32 id) { return m_reg.constructMessage(id); }

            HASH32 addPlugin(void* handle, const std::string& path, bool& exists) { return m_reg.addPlugin(handle, path, exists); }
            MutexLocker getPluginMutex(HASH32 id) { return m_reg.getPluginMutex(id); }
            void* checkPlugin(HASH32 id) { return m_reg.checkPlugin(id); }
            bool updatePlugin(HASH32 id, const std::string& path, void* handle) { return m_reg.updatePlugin(id, path, handle); }
            void cleanupPlugins() { m_reg.cleanupPlugins(); }

            //=====================SOCKETS========================
        };
    }  // namespace core
}  // namespace cerberus

#endif  // CERBERUS_CORE_CERBERUSCORE_H
