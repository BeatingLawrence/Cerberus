#ifndef CERBERUS_MESSAGE_SLOT_FLOATSLOT_H
#define CERBERUS_MESSAGE_SLOT_FLOATSLOT_H

#include "../../Cerberus_global.h"
#include "baseslot.h"

namespace cerberus
{
    namespace message
    {
        namespace slot
        {
            class CERBERUS_EXPORT FloatSlot : public BaseSlot
            {
               private:
                float m_value;

               public:
                static cerberus_slot create(float value = 0.0f);

                static cerberus_slot create(const FloatSlot& other);

                FloatSlot(float value = 0.0f);

                float value() const;

                void value(float value);
            };
        }  // namespace slot
    }      // namespace message
}  // namespace cerberus

#endif  // CERBERUS_MESSAGE_SLOT_FLOATSLOT_H
