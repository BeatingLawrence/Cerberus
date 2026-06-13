#ifndef RECORDABLE_H
#define RECORDABLE_H

#include <string>

#include "../Cerberus_global.h"
#include "../exception/exception.h"
#include "../types.h"

namespace crb
{
    class Cerberus;

    namespace core
    {
        class CerberusRegister;

        class Recordable
        {
            friend class ::crb::core::CerberusRegister;
            friend class ::crb::Cerberus;

           public:
            // Return a string containing the object type (and socket type if present) and ID,
            // like "Thread ID:123456"
            CERBERUS_EXPORT static std::string toStr(const Recordable& obj);

            // Same as above
            CERBERUS_EXPORT std::string toObjStr();

       private:
            HASH32 m_id;

            static std::string toThreadStr(const Recordable& obj);

       protected:
            CERBERUS_EXPORT Recordable();

       public:
            Recordable(const Recordable& other) = delete;

            CERBERUS_EXPORT virtual ~Recordable();

            // Register this instance in the Cerberus framework register.
            // After this call, all other registered objects will be capable of
            // getting a reference to this instance and use it
            CERBERUS_EXPORT virtual void checkIn(const std::string& name);

            // Un-register this instance from the Cerberus framework register.
            CERBERUS_EXPORT virtual void checkOut();

            // Checks if the object is registered
            CERBERUS_EXPORT bool isRegistered() const;

            // Returns the object ID
            CERBERUS_EXPORT HASH32 id() const;

            // Performs a dynamic cast of this object into T. Throws an exception if the cast is not possible
            // Checking the object type() before casting is a good practice
            template <class T>
            T* to_p()
            {
                T* casted = dynamic_cast<T*>(this);

                if (casted == nullptr)
                {
                    throw cInvalidCastExc("Unable co cast to %s", typeid(T).name());
                }

                return casted;
            }

            template <class T>
            T& to()
            {
                T* casted = dynamic_cast<T*>(this);

                if (casted == nullptr)
                {
                    throw cInvalidCastExc("Unable co cast to %s", typeid(T).name());
                }

                return *casted;
            }

            template <class T>
            const T* to_p() const
            {
                T* casted = dynamic_cast<T*>(this);

                if (casted == nullptr)
                {
                    throw cInvalidCastExc("Unable co cast to %s", typeid(T).name());
                }

                return casted;
            }

            template <class T>
            const T& to() const
            {
                T* casted = dynamic_cast<T*>(this);

                if (casted == nullptr)
                {
                    throw cInvalidCastExc("Unable co cast to %s", typeid(T).name());
                }

                return *casted;
            }
        };
    }  // namespace core
}  // namespace crb

#endif  // RECORDABLE_H
