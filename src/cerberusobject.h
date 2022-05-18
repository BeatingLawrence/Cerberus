#ifndef CERBERUSOBJECT_H
#define CERBERUSOBJECT_H

#include <cstdint>
#include <string>
#include <memory>
#include "exception/exceptioncatalog.h"
#include "./cerberus.h"
#include "./Cerberus_global.h"

namespace cerberus
{
    class CERBERUS_EXPORT CerberusObject
    {
        private:
            uint32_t m_id;

            uint32_t m_type;

            std::string m_name;

        protected:
            CerberusObject(uint32_t type, const std::string& name);

        public:
            virtual ~CerberusObject();

            //Returns the object ID
            uint32_t id() const;

            //Returns the object type
            uint32_t type() const;

            //Returns the object name
            std::string name() const;

            //Performs a dynamic cast of this object into T. An exception will be thrown if cast is invalid.
            template<class T> T* to()
            {
                T* casted = dynamic_cast<T*>(this);

                if(casted == nullptr)
                {
                    throw cerberusIllegalArgumentExc(Cerberus::strPrint("Unable co cast to %s", typeid(T).name()).c_str());
                }

                return casted;
            }
    };
}

#endif // CERBERUSOBJECT_H
