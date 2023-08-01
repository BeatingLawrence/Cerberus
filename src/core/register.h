#ifndef CERBERUS_REGISTER_H
#define CERBERUS_REGISTER_H

/*  This class holds all Cerberus registers, and generates the IDs of the new entries.
 *
 *  This class is not supposed to be used directly. An instance of this class
 *  will be created and managed by the CerberusFactory
 */

#include <cstdint>
#include <list>
#include <string>

#include "../mutex/mutex.h"

namespace cerberus
{
    class CerberusObject;

    class Register
    {
       private:
        std::list<CerberusObject*> m_objects;

        uint32_t findAvailableId();

        mutex::Mutex registerMutex;

       public:
        Register();

        Register(const Register& other) = delete;

        ~Register();

        // Add an object to the register and returns the ID.
        // Return an invalid ID if the registering failed
        uint32_t registerObject(CerberusObject* object);

        // Remove the object with given ID from the register.
        // Nothing happens if the ID does not exist
        void unregisterObject(uint32_t id);

        // Give a cerberus object from its name, or nullptr if it does not exist
        CerberusObject* objectByName(const std::string& name);

        // Give a cerberus object from its ID, or nullptr if it does not exist
        CerberusObject* objectById(uint32_t id);
    };

}  // namespace cerberus

#endif  // CERBERUS_REGISTER_H
