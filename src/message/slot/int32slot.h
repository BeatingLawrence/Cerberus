#ifndef CERBERUS_MESSAGE_SLOT_INT32SLOT_H
#define CERBERUS_MESSAGE_SLOT_INT32SLOT_H

#include "../../Cerberus_global.h"
#include "baseslot.h"

namespace cerberus
{
    namespace message
    {
        namespace slot
        {
            class CERBERUS_EXPORT Int32Slot : public BaseSlot
            {
               private:
                int32_t m_value;

               public:
                static cerberus_slot create(int32_t value = 0);

                static cerberus_slot create(const Int32Slot& other);

                Int32Slot(int32_t value = 0);

                int32_t value() const;

                void value(int32_t value);
            };
        }  // namespace slot
    }      // namespace message
}  // namespace cerberus

#endif  // CERBERUS_MESSAGE_SLOT_INT32SLOT_H
