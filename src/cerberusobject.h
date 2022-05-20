#ifndef CERBERUSOBJECT_H
#define CERBERUSOBJECT_H

#include <cstdint>
#include <string>
#include <memory>
#include "exception/exceptioncatalog.h"
#include "./core/cerberusutils.h"

namespace cerberus
{
    class CerberusObject
    {
        public:
            enum ObjectType : uint8_t
            {
                OT_Thread,
                OT_MessageTemplate,
                //add more types here..
            };

        private:
            uint32_t m_id;

            ObjectType m_type;

            std::string m_name;

        protected:
            CerberusObject(ObjectType type, const std::string& name);

        public:
            virtual ~CerberusObject();

            //Returns the object ID
            uint32_t id() const;

            //Returns the object type
            ObjectType type() const;

            //Returns the object name
            std::string name() const;

            //Performs a dynamic cast of this object into T. An exception will be thrown if cast is invalid.
            template<class T> T* to()
            {
                T* casted = dynamic_cast<T*>(this);

                if(casted == nullptr)
                {
                    throw cerberusIllegalArgumentExc(core::CerberusUtils::strPrint("Unable co cast to %s", typeid(T).name()).c_str());
                }

                return casted;
            }
    };
}

#endif // CERBERUSOBJECT_H
