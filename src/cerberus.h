#ifndef CERBERUS_H
#define CERBERUS_H

#include "Cerberus_global.h"
#include "core/cerberuslog.h"
#include "data/data.h"        // IWYU pragma: export
#include "data/filesystem/inidatafile.h"
#include "log.h"              // IWYU pragma: export
#include "message/message.h"  // IWYU pragma: export
#include "message/slot.h"     // IWYU pragma: export
#include "time/datetime.h"    // IWYU pragma: export
#include "time/timeframe.h"   // IWYU pragma: export
#include "types.h"

namespace cerberus
{
    namespace core
    {
        class CerberusCore;
        class CerberusRegister;
        class CerberusLog;
        class LibLoader;

        template <class T>
        struct FrameworkLock
        {
            struct FrameworkLocker
            {
                FrameworkLock& lock;

                FrameworkLocker(FrameworkLock& lock)
                    : lock(lock) {};

                ~FrameworkLocker() { lock.end(); };
            };

            friend FrameworkLocker;

            std::atomic<bool> ready;
            std::atomic<int> usage;
            T* data;

            FrameworkLock()
                : ready(false),
                  usage(0),
                  data(nullptr) {};

            FrameworkLock(const FrameworkLock& other) = delete;

           private:
            void begin() { usage.fetch_add(1); }
            void end() { usage.fetch_sub(1); }

           public:
            FrameworkLocker getLocker()
            {
                begin();
                return FrameworkLocker(*this);
            }

            void wait()
            {
                while (usage.load() != 0)
                {
                    asm("nop");
                }
            }

            void construct()
            {
                if (!data) data = new T;
                ready.store(true);
            }

            void destroy()
            {
                ready.store(false);
                wait();
                delete data;
                data = nullptr;
            }

            bool isReady() { return ready.load(); };

            void isReadySevere()
            {
                if (!isReady()) throw cUsageErrorExc("bad init usage");
            };
        };

        struct FrameworkData
        {
            core::FrameworkLock<core::CerberusLog> log;
            core::FrameworkLock<core::CerberusCore> core;

            void construct(const CerberusInitConf& conf);
            void destroy();

            void start();
            void stop();
        };
    }  // namespace core

    class Timer;
    class Alarm;

    class CERBERUS_EXPORT Cerberus
    {
        friend class ::cerberus::core::CerberusCore;
        friend class ::cerberus::core::Recordable;
        friend class ::cerberus::core::CerberusLog;
        friend class ::cerberus::core::LibLoader;
        friend class ::cerberus::Timer;
        friend class ::cerberus::Alarm;

       private:
        static core::FrameworkData framework;

        static msg_ptr stdTemplate(HASH32 id);

        // ======================Private Register===========================

        // Register a given object and return its id
        // Return an invalid ID if the registering failed
        static void registerObj(core::Recordable* object, const std::string& name);

        // Unregister an object by its id
        static void unregisterObj(HASH32 id);

        // Directly send a message to a cerberus object
        static OpRes sendMsgToObj(HASH32 id, msg_ptr& msg);

        // Directly send a message to a cerberus object (deep-copy, copy-late)
        static OpRes sendMsgToObj_deep(HASH32 id, const msg_ptr& msg);

        // =======================Plugin manager============================

        // Add a plugin handle to the register. If the handle already exixst, exists is true
        // The new (or found) ID is returned
        static HASH32 addPlugin(void* handle, const std::string& path, bool& exists);

        // Get the mutexlocker of a loaded shared object. The mutex is locked before return
        static MutexLocker getPluginMutex(HASH32 id);

        // Return the requested handle if it is registered, otherwise nullptr
        static void* checkPlugin(HASH32 id);

        // Replaces data of an existing plugin. Returns false if id does not exist, true otherwise
        static bool updatePlugin(HASH32 id, const std::string& path, void* handle);

        // =======================Event scheduler===========================

        // Start a timer
        static void startTimer(TimerData& data);

        // Stop a timer and remove it from references
        static void stopTimer(std::atomic_bool& bit);

       public:
        Cerberus() = delete;

        // Perform the init sequence of the Cerberus framework
        // This operation must precede any others
        // The default parameter will use default settings
        static void init(const CerberusInitConf& parms = cerberusDefaultParms());

        // Perform the de-init sequence of the Cerberus framework
        static void deinit();

        // Return a working default set of init parameters
        static CerberusInitConf cerberusDefaultParms();

        // Return the version of the Cerberus framework
        static CerbVersion cerberusVersion();

        // Log method
        static void log(const std::string& str, LogLevel logLevel = LL_Info,
                        const std::string& author = std::string(), bool application = true);

        // Configuration file (IniDataFile) access
        static void setFileName(const std::string& fileName);
        static OpRes load();
        static bool exists(const std::string& key, const std::string& section = MAIN_SECTION);
        static IniDataType type(const std::string& key, const std::string& section = MAIN_SECTION);
        static bool isType(const std::string& key, IniDataType type);
        static OpRes rewrite();

        static OpRes write_string(const std::string& key, const std::string& value,
                                  const std::string& section = MAIN_SECTION);
        static OpRes write_integer(const std::string& key, int64_t value, const std::string& section = MAIN_SECTION);
        static OpRes write_double(const std::string& key, double value, const std::string& section = MAIN_SECTION);
        static OpRes write_bool(const std::string& key, bool value, const std::string& section = MAIN_SECTION);

        static OpRes enforce_string(const std::string& key, const std::string& value,
                                    const std::string& section = MAIN_SECTION);
        static OpRes enforce_integer(const std::string& key, int64_t value,
                                     const std::string& section = MAIN_SECTION);
        static OpRes enforce_double(const std::string& key, double value,
                                    const std::string& section = MAIN_SECTION);
        static OpRes enforce_bool(const std::string& key, bool value,
                                  const std::string& section = MAIN_SECTION);

        static StringOpRes read_string(const std::string& key, const std::string& section = MAIN_SECTION);
        static IntOpRes read_integer(const std::string& key, const std::string& section = MAIN_SECTION);
        static FloatOpRes read_double(const std::string& key, const std::string& section = MAIN_SECTION);
        static BoolOpRes read_bool(const std::string& key, const std::string& section = MAIN_SECTION);

        // send with deep copy
        static OpRes send_deep(const msg_ptr& message, HASH32 recipientID = CERBERUS_INVALID_ID);
        static OpRes send_deep(const msg_ptr& message, const std::string& recipient);
        static OpRes send_deep(const msg_ptr& message, HASH32 recipientID, HASH32 channel_in);
        static OpRes send_deep(const msg_ptr& message, const std::string& recipient, HASH32 channel_in);

        // send using std::move
        static OpRes send(msg_ptr& message, HASH32 recipientID = CERBERUS_INVALID_ID);
        static OpRes send(msg_ptr& message, const std::string& recipient);
        static OpRes send(msg_ptr& message, HASH32 recipientID, HASH32 channel_in);
        static OpRes send(msg_ptr& message, const std::string& recipient, HASH32 channel_in);

        // ======================Public Register===========================

        // Retrieve an object ID by its name
        static HASH32 idByName(const std::string& name);

        // ===========================Factory==============================

        // Add a template of the given message to the register
        static OpRes registerTemplate(const msg_ptr& tmplt);

        // Factory of messages. A call to this method will return an empty but structured message.
        // This method will return an invalid message if ID was not found, or it will
        // throw an exception if the provided ID is not valid (invalid id or in reserved range)
        static msg_ptr constructMessage(HASH32 id);

        // Factory of messages. A call to this method will return an empty but structured message.
        // This method will return an invalid message if name was not found,
        // or if it's not a Message name.
        static msg_ptr constructMessage(const std::string& name);
    };
}  // namespace cerberus

#endif  // CERBERUS_H
