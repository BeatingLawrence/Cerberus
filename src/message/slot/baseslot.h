#ifndef CERBERUS_MESSAGE_SLOT_BASESLOT_H
#define CERBERUS_MESSAGE_SLOT_BASESLOT_H

/*  This is the Slots base class.
 *  This class cannot be instantiated, but it must be extended by a derived class which contains the data itself.
 *
 *  Data type is specified in the SlotType enumeration
 *
 *  The ID of the Slot is chosen by the user while creating a message, and it has no effect on Cerberus dispatching and message management.
 */

#include <memory>
#include "../../define.h"
#include "../../Cerberus_global.h"
#include "../../cerberus.h"
#include "../../exception/exceptioncatalog.h"
#include "../../core/cerberusutils.h"

namespace cerberus
{
    namespace message
    {
        namespace slot
        {
            class CERBERUS_EXPORT BaseSlot
            {
                private:
                    SlotType m_dataType;

                    uint32_t m_id;

                protected:
                    //Constructs a BaseSlot with a given type
                    BaseSlot(SlotType type);

                    //Copy-constructs another BaseSlot
                    BaseSlot(const BaseSlot& other);

                public:
                    BaseSlot() = delete;

                    virtual ~BaseSlot();

                    //Gets the type
                    SlotType type() const;

                    //Gets the ID, useful for dispatching
                    uint32_t id() const;

                    //Sets an ID
                    void setId(uint32_t id);

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

                    //Performs a dynamic cast of from into a shared_ptr of type T, caring for the instance counter.
                    //An exception will be thrown if cast is invalid.
                    template<class T> static std::shared_ptr<T> toShared(const cerberus_slot& from)
                    {
                        std::shared_ptr<T> casted = std::dynamic_pointer_cast<T, BaseSlot>(from);

                        if(*casted == nullptr)
                        {
                            throw cerberusIllegalArgumentExc(core::CerberusUtils::strPrint("Unable co cast to shared_ptr of %s", typeid(T).name()).c_str());
                        }

                        return casted;
                    }
            };
        }
    }
}

#endif // CERBERUS_MESSAGE_SLOT_BASESLOT_H
