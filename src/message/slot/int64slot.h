#ifndef CERBERUS_MESSAGE_SLOT_INT64SLOT_H
#define CERBERUS_MESSAGE_SLOT_INT64SLOT_H

#include "../../Cerberus_global.h"
#include "baseslot.h"

namespace cerberus
{
    namespace message
    {
        namespace slot
        {
            class CERBERUS_EXPORT Int64Slot : public BaseSlot
            {
               private:
                int64_t m_value;

               public:
                static cerberus_slot create(int64_t value = 0);

                static cerberus_slot create(const Int64Slot& other);

                Int64Slot(int64_t value = 0);

                int64_t value() const;

                void value(int64_t value);
            };
        }  // namespace slot
    }      // namespace message
}  // namespace cerberus

#endif  // CERBERUS_MESSAGE_SLOT_INT64SLOT_H
