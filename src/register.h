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
 *  will be created and managed by the CerberusFactory
 */

#include <list>
#include "./Cerberus_global.h"
#include "./mutex/mutex.h"

namespace cerberus
{
    class CerberusObject;

    class Register
    {
        private:
            std::list<CerberusObject*> m_objects;

            uint32_t _findAvailableID_objects();

            mutable mutex::Mutex m_objectMutex;

        public:
            Register();

            ~Register();

            Register(const Register& other) = delete;

            uint32_t registerCerberusObject(CerberusObject* object);

            void unregisterCerberusObject(uint32_t id);

            CerberusObject* cerberusObjectByName(const std::string& name) const;

            CerberusObject* cerberusObjectByID(uint32_t id) const;

            void freeMemory();  //Not protected by mutex!
    };

} // namespace cerberus

#endif // CERBERUS_REGISTER_H
