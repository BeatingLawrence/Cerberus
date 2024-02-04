#ifndef CERBERUS_MESSAGE_SLOT_BOOLSLOT_H
#define CERBERUS_MESSAGE_SLOT_BOOLSLOT_H

#include "../../Cerberus_global.h"
#include "baseslot.h"

namespace cerberus
{
    namespace message
    {
        namespace slot
        {
            class CERBERUS_EXPORT BoolSlot : public BaseSlot
            {
               private:
                bool m_value;

               public:
                static cerberus_slot create(bool value = 0);

                static cerberus_slot create(const BoolSlot& other);

                BoolSlot(bool value = 0);

                bool value() const;

                void value(bool value);
            };
        }  // namespace slot
    }      // namespace message
}  // namespace cerberus

#endif  // CERBERUS_MESSAGE_SLOT_BOOLSLOT_H
