#ifndef CERBERUSOBJECT_H
#define CERBERUSOBJECT_H

#include <cstdint>
#include <string>

#include "../Cerberus_global.h"
#include "../exception/exception.h"
#include "../types.h"

namespace cerberus
{
    class Cerberus;

    namespace core
    {
        class CerberusRegister;

        class CERBERUS_EXPORT CerberusObject
        {
            friend class ::cerberus::core::CerberusRegister;
            friend class ::cerberus::Cerberus;

           public:
            enum ObjectType : uint8_t
            {
                COBJ_Invalid,
                COBJ_Thread,
                COBJ_MessageTmplt,
                COBJ_Socket,
                // add more types here..
            };

            enum SocketType : uint8_t
            {
                Socket_None,
                Socket_UDP,
                Socket_TCP,
                Socket_TCPP2P,
                Socket_HTTP,
                Socket_ICMP,
                Socket_IPC,
            };

            // Return a string containing the object type (and socket type if present) and ID,
            // like "Thread ID:123456" or "Socket TCP ID:123456"
            static std::string toStr(const CerberusObject& obj);

            // Same as above
            std::string toObjStr();

           private:
            HASH32 m_id;

            ObjectType m_type;

            std::string m_name;

            SocketType m_socketType;

            bool m_cerbManaged;

            static std::string toThreadStr(const CerberusObject& obj);

           protected:
            CerberusObject(ObjectType type, const std::string& name = std::string());

            CerberusObject(SocketType type, const std::string& name = std::string());

           public:
            CerberusObject() = delete;

            virtual ~CerberusObject();

            // Register this instance in the Cerberus framework register.
            // After this call, all other registered objects will be capable of
            // getting a reference to this instance and use it
            void checkIn();

            // Un-register this instance from the Cerberus framework register.
            void checkOut();

            // Checks if the object is registered
            bool isRegistered() const;

            // Returns the object ID
            HASH32 id() const;

            // Returns the object type
            ObjectType type() const;

            // Returns the socket type
            SocketType socketType() const;

            // Returns the object name
            std::string name() const;

            // Performs a dynamic cast of this object into T. Throws an exception if the cast is not possible
            // Checking the object type() before casting is a good practice
            template <class T>
            T* to_p()
            {
                T* casted = dynamic_cast<T*>(this);

                if (casted == nullptr)
                {
                    throw cerberusInvalidCastExc("Unable co cast to %s", typeid(T).name());
                }

                return casted;
            }

            template <class T>
            T& to()
            {
                T* casted = dynamic_cast<T*>(this);

                if (casted == nullptr)
                {
                    throw cerberusInvalidCastExc("Unable co cast to %s", typeid(T).name());
                }

                return *casted;
            }

            template <class T>
            const T* to_p() const
            {
                T* casted = dynamic_cast<T*>(this);

                if (casted == nullptr)
                {
                    throw cerberusInvalidCastExc("Unable co cast to %s", typeid(T).name());
                }

                return casted;
            }

            template <class T>
            const T& to() const
            {
                T* casted = dynamic_cast<T*>(this);

                if (casted == nullptr)
                {
                    throw cerberusInvalidCastExc("Unable co cast to %s", typeid(T).name());
                }

                return *casted;
            }
        };
    }  // namespace core
}  // namespace cerberus

#endif  // CERBERUSOBJECT_H
