#ifndef CERBERUSOBJECT_H
#define CERBERUSOBJECT_H

#include <cstdint>
#include <memory>
#include <string>

#include "src/Cerberus_global.h"
#include "src/core/cerberuslog.h"
#include "src/exception/exceptioncatalog.h"

namespace cerberus
{
    class CERBERUS_EXPORT CerberusObject
    {
       public:
        enum ObjectType : uint8_t
        {
            Thread,
            MessageTemplate,
            Socket,
            // add more types here..
        };

        enum SocketType : uint8_t
        {
            Socket_None,
            Socket_UDP,
            Socket_TCP,
            Socket_TCPP2P,
            Socket_HTTP,
            Socket_WEB,
            Socket_FTP,
            Socket_ICMP,
            Socket_IPC,
        };

        // Return a string containing the object type (and socket type if present) and ID,
        // like "Thread ID:123456" or "Socket TCP ID:123456"
        static std::string toStr(const CerberusObject& obj);

        // Same as above but non static version
        std::string toObjStr();

       private:
        uint32_t m_id;

        ObjectType m_type;

        std::string m_name;

        SocketType m_socketType;

       protected:
        CerberusObject(ObjectType type, const std::string& name = std::string());

        CerberusObject(SocketType type, const std::string& name = std::string());

       public:
        virtual ~CerberusObject();

        // Checks if the object is valid
        bool isObjValid();

        // Returns the object ID
        uint32_t id() const;

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
                debug("failed to cast Cerberus object %u to %s", m_id, typeid(T).name());
                throw cerberusInvalidCastExc("Unable co cast to %s", typeid(T).name());
            }

            return casted;
        }

        // Performs a dynamic cast of this object into T. Throws an exception if the cast is not possible
        // Checking the object type() before casting is a good practice
        template <class T>
        T& to()
        {
            T* casted = dynamic_cast<T*>(this);

            if (casted == nullptr)
            {
                debug("failed to cast Cerberus object %u to %s", m_id, typeid(T).name());
                throw cerberusInvalidCastExc("Unable co cast to %s", typeid(T).name());
            }

            return *casted;
        }
    };
}  // namespace cerberus

#endif  // CERBERUSOBJECT_H
