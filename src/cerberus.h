#ifndef CERBERUS_H
#define CERBERUS_H

#include "Cerberus_global.h"
#include "core/cerberuscore.h"

namespace cerberus
{
    namespace time
    {
        class Timer;
    }
    class CERBERUS_EXPORT Cerberus : public core::CerberusCore
    {
        friend class ::cerberus::CerberusObject;
        friend class ::cerberus::time::Timer;

       private:
        Cerberus();
        Cerberus(const Cerberus& other)  = delete;
        Cerberus(const Cerberus&& other) = delete;
        static Cerberus& instance();

        bool m_initFlag;

        mutex::Mutex m_mutex;

        ~Cerberus();

       public:
        // Performs the init sequence of the Cerberus framework. This operation must precede any others
        // Specify nullptr as parameter to use default settings
        static void init(const CerberusInitParms& parms);

        // Performs the init sequence of the Cerberus framework using default parameters.
        // This call is equal to init(cerberusDefaultParms());
        static void init();

        // Performs the de-init sequence of the Cerberus framework. (TBC)
        static void deinit();

        // Sends a message
        static void send(message::cerberus_message message);

        // Returns a working default set of init parameters
        static CerberusInitParms cerberusDefaultParms();

        // Returns the version of the cerberus framework
        static std::string cerberusVersion();
    };
}  // namespace cerberus

#endif  // CERBERUS_H
