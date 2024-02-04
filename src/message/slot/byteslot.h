#ifndef CERBERUS_MESSAGE_SLOT_BYTESLOT_H
#define CERBERUS_MESSAGE_SLOT_BYTESLOT_H

#include "../../Cerberus_global.h"
#include "baseslot.h"

namespace cerberus
{
    namespace message
    {
        namespace slot
        {
            class CERBERUS_EXPORT ByteSlot : public BaseSlot
            {
               private:
                BYTE m_value;

               public:
                static cerberus_slot create(BYTE value = 0);

                static cerberus_slot create(const ByteSlot& other);

                ByteSlot(BYTE value = 0);

                BYTE value() const;

                void value(BYTE value);
            };
        }  // namespace slot
    }      // namespace message
}  // namespace cerberus

#endif  // CERBERUS_MESSAGE_SLOT_BYTESLOT_H
