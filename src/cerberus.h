#ifndef CERBERUS_H
#define CERBERUS_H

#include "Cerberus_global.h"
#include "core/cerberuslog.h"
#include "log.h"
#include "message/message.h"
#include "message/messagetemplate.h"
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
        class CerberusFactory;
        class CerberusLog;
        class LibLoader;
    }  // namespace core

    namespace time
    {
        class Timer;
    }

    struct SingularCerberusData
    {
       private:
        std::atomic_flag logWorking, regWorking, coreWorking;  // init order

        core::CerberusLog* log;
        core::CerberusRegister* reg;
        core::CerberusCore* core;

       public:
        SingularCerberusData()
            : logWorking(false),
              regWorking(false),
              coreWorking(false),
              log(nullptr),
              reg(nullptr),
              core(nullptr){};

        void constructLog();
        void constructReg();
        void constructCore();

        void destroyLog();
        void destroyReg();
        void destroyCore();

        core::CerberusLog* getLog();
        core::CerberusRegister* getReg();
        core::CerberusCore* getCore();
    };

    class CERBERUS_EXPORT Cerberus
    {
        friend class ::cerberus::core::CerberusCore;
        friend class ::cerberus::core::CerberusObject;
        friend class ::cerberus::core::CerberusFactory;
        friend class ::cerberus::core::CerberusLog;
        friend class ::cerberus::core::LibLoader;
        friend class ::cerberus::time::Timer;

       private:
        static SingularCerberusData singularCerbData;

        // ======================Private Register===========================

        // Register a given object and return its id
        // Return an invalid ID if the registering failed
        static void registerObj(core::CerberusObject* object);

        // Unregister an object by its id
        static void unregisterObj(uint32_t id);

        // Directly send a message to a cerberus object
        static void sendMsgToObj(uint32_t id, cerberus_message msg);

        // =======================Plugin manager============================

        // Add a plugin handle to the register. If the handle already exixst, exists is true
        // The new (or found) ID is returned
        static uint32_t addPlugin(void* handle, const std::string& path, bool& exists);

        // Get the mutexlocker of a loaded shared object. The mutex is locked before return
        static mutex::MutexLocker getPluginMutex(uint32_t id);

        // Return the requested handle if it is registered, otherwise nullptr
        static void* checkPlugin(uint32_t id);

        // Replaces data of an existing plugin. Returns false if id does not exist, true otherwise
        static bool updatePlugin(uint32_t id, const std::string& path, void* handle);

        // =======================Event scheduler===========================

        // Start a new periodic timer that will fire every t time
        static void startTimer(std::atomic_bool& bit, time::TimeFrame t, timerCallback callback);

        // Start a new periodic timer that will fire at d (the first time) and then, every t time
        static void startTimer(std::atomic_bool& bit, time::DateTime d, time::TimeFrame t, timerCallback callback);

        // Start a new one-shot timer that will fire at d and then it will be removed from the references
        // as stopTimer() were called
        static void startTimer(std::atomic_bool& bit, time::DateTime d, timerCallback callback);

        // Stop a timer and remove it from references
        static void stopTimer(std::atomic_bool& bit);

       public:
        Cerberus() = delete;

        // Perform the init sequence of the Cerberus framework
        // This operation must precede any others
        // The default parameter will use default settings
        static void init(const CerberusInitParms& parms = cerberusDefaultParms());

        // Perform the de-init sequence of the Cerberus framework
        static void deinit();

        // Send a message
        static void send(cerberus_message message);

        // Send a message ignoring the destination of message and using id instead
        static void send(cerberus_message message, uint32_t id);

        // Send a message ignoring the destination of message and using the id of the named object instead
        static void send(cerberus_message message, const std::string& name);

        // Return a working default set of init parameters
        static CerberusInitParms cerberusDefaultParms();

        // Return the version of the Cerberus framework
        static CerbVersion cerberusVersion();

        // Log method
        static void log(const std::string& str, LogLevel logLevel = LL_Info, const std::string& author = std::string(), bool application = true);

        // ======================Public Register===========================

        // Retrieves an object ID by its name
        static uint32_t objIdByName(const std::string& name);

        // ===========================Factory==============================

        // Retrieves a MessageTemplate by its ID
        static message::MessageTemplate msgTemplateById(uint32_t id);

        // Retrieves a MessageTemplate by its name
        static message::MessageTemplate msgTemplateByName(const std::string& name);

        // Adds a template of the given message to the register, returning the chosen typeID
        static uint32_t registerMessage(const message::Message& message, const std::string& name = std::string());

        // Factory of messages. A call to this method will return an empty but structured message.
        // This method will return an invalid message if ID was not found,
        // or if it's not a Message ID.
        static cerberus_message messageConstruct(uint32_t id);

        // Factory of messages. A call to this method will return an empty but structured message.
        // This method will return an invalid message if name was not found,
        // or if it's not a Message name.
        static cerberus_message messageConstruct(const std::string& name);

        // Construct a standard message. See the StandardMessage enum
        static cerberus_message standardMessageConstruct(StandardMessage type);
    };
}  // namespace cerberus

#endif  // CERBERUS_H
