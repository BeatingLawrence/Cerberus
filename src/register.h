#ifndef CERBERUS_REGISTER_H
#define CERBERUS_REGISTER_H

/*  This class holds all Cerberus registers, and generates the IDs of the new entries.
 *
 *  Registers are:
 *      1. Message templates register
 *      2. Threads register
 *      3. Sockets register
 *      ...
 *
 *  This class is not supposed to be used directly. An instance of this class
 *  will be created and managed by the Cerberus provider
 */

#include <list>
#include "./Cerberus_global.h"
#include "./mutex/mutex.h"

namespace cerberus
{
    class CerberusObject;

    class CERBERUS_EXPORT Register
    {
        private:
            std::list<CerberusObject*> m_objects;

            uint32_t _findAvailableID_objects();

            mutable mutex::Mutex m_objectMutex;

        public:
            Register();

            Register(const Register& other) = delete;

            uint32_t registerCerberusObject(CerberusObject* object);

            void unregisterCerberusObject(uint32_t id);

            CerberusObject* cerberusObjectByName(const std::string& name) const;

            CerberusObject* cerberusObjectByID(uint32_t id) const;

            bool isEmpty() const;

            void freeMemory();
    };

} // namespace cerberus

#endif // CERBERUS_REGISTER_H
