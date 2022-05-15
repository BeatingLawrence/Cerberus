#ifndef CERBERUSOBJECT_H
#define CERBERUSOBJECT_H

#include <cstdint>
#include <string>
#include <memory>

namespace cerberus
{
    typedef std::shared_ptr<class CerberusObject> cerberus_object;

    class CerberusObject
    {
        private:
            uint32_t m_id;

            uint32_t m_type;

            std::string m_name;

        protected:
            CerberusObject(uint32_t type, const std::string& name);

        public:
            //Returns the object ID
            uint32_t id() const;

            //Returns the object type
            uint32_t type() const;

            //Returns the object name
            std::string name() const;

            //Performs a dynamic cast of this object into T. An exception will be thrown if cast is invalid.
            template<class T> T* to();
    };
}

#endif // CERBERUSOBJECT_H
