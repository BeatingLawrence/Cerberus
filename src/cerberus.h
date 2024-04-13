#ifndef CERBERUS_H
#define CERBERUS_H

#include "Cerberus_global.h"
#include "core/cerberuslog.h"
#include "data/data.h"        // IWYU pragma: export
#include "log.h"              // IWYU pragma: export
#include "message/message.h"  // IWYU pragma: export
#include "message/messagetemplate.h"
#include "message/slot/slot.h"  // IWYU pragma: export
#include "time/datetime.h"
#include "time/timeframe.h"
#include "types.h"

namespace cerberus
{
    namespace core
    {
        class CerberusObject;
        class CerberusCore;
        class CerberusRegister;
        class CerberusLog;
        class LibLoader;
    }  // namespace core

    class Timer;

    template <class T>
    struct FrameworkLock
    {
        struct FrameworkLocker
        {
            FrameworkLock& lock;

            FrameworkLocker(FrameworkLock& lock)
                : lock(lock){};

            ~FrameworkLocker() { lock.end(); };
        };

        friend FrameworkLocker;

        std::atomic<bool> ready;
        std::atomic<int> usage;
        T* data;

        FrameworkLock()
            : ready(false),
              usage(0),
              data(nullptr){};

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
        FrameworkLock<core::CerberusLog> log;
        FrameworkLock<core::CerberusRegister> reg;
        FrameworkLock<core::CerberusCore> core;

        void construct(const CerberusInitConf& conf);
        void destroy();

        void start();
        void stop();
    };

    class CERBERUS_EXPORT Cerberus
    {
        friend class ::cerberus::core::CerberusCore;
        friend class ::cerberus::core::CerberusObject;
        friend class ::cerberus::core::CerberusLog;
        friend class ::cerberus::core::LibLoader;
        friend class ::cerberus::Timer;

       private:
        static FrameworkData framework;

        // ======================Private Register===========================

        // Register a given object and return its id
        // Return an invalid ID if the registering failed
        static void registerObj(core::CerberusObject* object);

        // Unregister an object by its id
        static void unregisterObj(HASH32 id);

        // Directly send a message to a cerberus object
        static void sendMsgToObj(HASH32 id, cerberus_message msg);

        // Check if the given object is a Cerberus-managed object
        static BoolOpRes isCerbManaged(HASH32 id);

        static OpResData<core::CerberusObject*> rawObjById(HASH32 id);

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

        // Start a new periodic timer that will fire every t time
        static void startTimer(std::atomic_bool& bit, TimeFrame t, timerCallback callback);

        // Start a new periodic timer that will fire at d (the first time) and then, every t time
        static void startTimer(std::atomic_bool& bit, DateTime d, TimeFrame t, timerCallback callback);

        // Start a new one-shot timer that will fire at d and then it will be removed from the
        // references as stopTimer() were called
        static void startTimer(std::atomic_bool& bit, DateTime d, timerCallback callback);

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

        // Send a message
        static void send(cerberus_message message);

        // Send a message ignoring the destination of message and using id instead
        static void send(cerberus_message message, HASH32 recipientID);

        // Send a message using the id of the given named object
        static void send(cerberus_message message, const std::string& recipient);

        // Return a working default set of init parameters
        static CerberusInitConf cerberusDefaultParms();

        // Return the version of the Cerberus framework
        static CerbVersion cerberusVersion();

        // Log method
        static void log(const std::string& str, LogLevel logLevel = LL_Info,
                        const std::string& author = std::string(), bool application = true);

        // ======================Public Register===========================

        // Retrieves an object ID by its name
        static HASH32 objIdByName(const std::string& name);

        // ===========================Factory==============================

        // Retrieves a MessageTemplate by its ID
        static MessageTemplate msgTemplateById(HASH32 id);

        // Retrieves a MessageTemplate by its name
        static MessageTemplate msgTemplateByName(const std::string& name);

        // Adds a template of the given message to the register, returning the chosen typeID
        static HASH32 registerMessage(const Message& message, const std::string& name = std::string());

        // Factory of messages. A call to this method will return an empty but structured message.
        // This method will return an invalid message if ID was not found, or it will
        // throw an exception if the provided ID is not valid (invalid id or in reserved range)
        static cerberus_message messageConstruct(HASH32 id);

        // Factory of messages. A call to this method will return an empty but structured message.
        // This method will return an invalid message if name was not found,
        // or if it's not a Message name.
        static cerberus_message messageConstruct(const std::string& name);

        // Create a new CerberusObject into the Cerberus memory space.
        // The Cerberus Framework has the ownership of the object and will manage
        // its existance in memory. The application will be able to interact with it
        // sending messages to it.
        static HASH32 createThread(const std::string& name);
        static HASH32 createSocket(core::CerberusObject::SocketType socketType, const std::string& name);
    };
}  // namespace cerberus

#endif  // CERBERUS_H
