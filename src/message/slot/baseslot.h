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

#include "../../Cerberus_global.h"
#include "../../core/cerberusutils.h"
#include "../../exception/exception.h"
#include "../../types.h"

namespace cerberus
{
    namespace message
    {
        namespace slot
        {

            class CERBERUS_EXPORT BaseSlot
            {
               private:
                SlotType m_type;

                std::string m_name;

               protected:
                BaseSlot(SlotType type, const std::string& name = std::string());

               public:
                BaseSlot() = delete;

                virtual ~BaseSlot();

                // Get the type
                SlotType type() const;

                // Get the name
                std::string name() const;

                // Perform a dynamic cast of this object into T. An exception will be thrown if cast is invalid.
                template <class T>
                T* to()
                {
                    T* casted = dynamic_cast<T*>(this);

                    if (casted == nullptr)
                    {
                        throw cerberusIllegalArgExc(core::CerberusUtils::strPrint("Unable co cast to %s", typeid(T).name()).c_str());
                    }

                    return casted;
                }

                // Perform a dynamic cast of from into a shared_ptr of type T, caring for the instance counter.
                // An exception will be thrown if cast is invalid.
                template <class T>
                static std::shared_ptr<T> toShared(const cerberus_slot& from)
                {
                    std::shared_ptr<T> casted = std::dynamic_pointer_cast<T, BaseSlot>(from);

                    if (casted == nullptr)
                    {
                        throw cerberusIllegalArgExc(core::CerberusUtils::strPrint("Unable co cast to shared_ptr of %s", typeid(T).name()).c_str());
                    }

                    return casted;
                }
            };
        }  // namespace slot
    }      // namespace message
}  // namespace cerberus

#endif  // CERBERUS_MESSAGE_SLOT_BASESLOT_H
