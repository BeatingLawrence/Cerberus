#ifndef CERBERUS_H
#define CERBERUS_H

#include <string>
#include "Cerberus_global.h"
#include "./core/cerberuscore.h"
#include "./core/cerberuslog.h"

namespace cerberus
{
    class CERBERUS_EXPORT Cerberus : public core::CerberusCore
    {
            friend class ::cerberus::CerberusObject;

        private:
            Cerberus();

            Cerberus(const Cerberus& other) = delete;
            Cerberus(const Cerberus&& other) = delete;
            void operator=(const Cerberus& other) = delete;

            static Cerberus* _instance();

            bool m_initFlag;

            mutex::Mutex m_mutex;

            ~Cerberus();

        public:
            //Performs the init sequence of the Cerberus framework. This operation must precede any others [TRANSFERS: NO]
            static void init(const CerberusInitParms* parms);

            //Performs the de-init sequence of the Cerberus framework. (TBC)
            static void deinit();

            //Sends a message
            static void send(message::cerberus_message message);

            //Returns a working default set of init parameters [TRANSFERS: YES]
            static CerberusInitParms* cerberusDefaultParms();

            //Returns the version of the cerberus framework
            static std::string cerberusVersion();
    };
}

#endif // CERBERUS_H
