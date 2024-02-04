#ifndef CERBERUS_MESSAGE_SLOT_VOIDPSLOT_H
#define CERBERUS_MESSAGE_SLOT_VOIDPSLOT_H

#include "../../Cerberus_global.h"
#include "baseslot.h"

namespace cerberus
{
    namespace message
    {
        namespace slot
        {
            class CERBERUS_EXPORT VoidPSlot : public BaseSlot
            {
               private:
                void* m_value;

               public:
                static cerberus_slot create(void* value = nullptr);

                static cerberus_slot create(const VoidPSlot& other);

                VoidPSlot(void* value = nullptr);

                void* value() const;

                void value(void* value);
            };
        }  // namespace slot
    }      // namespace message
}  // namespace cerberus

#endif  // CERBERUS_MESSAGE_SLOT_VOIDPSLOT_H
