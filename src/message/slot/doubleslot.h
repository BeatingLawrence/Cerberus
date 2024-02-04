#ifndef CERBERUS_MESSAGE_SLOT_DOUBLESLOT_H
#define CERBERUS_MESSAGE_SLOT_DOUBLESLOT_H

#include "../../Cerberus_global.h"
#include "baseslot.h"

namespace cerberus
{
    namespace message
    {
        namespace slot
        {
            class CERBERUS_EXPORT DoubleSlot : public BaseSlot
            {
               private:
                double m_value;

               public:
                static cerberus_slot create(double value = 0.0f);

                static cerberus_slot create(const DoubleSlot& other);

                DoubleSlot(double value = 0.0f);

                double value() const;

                void value(double value);
            };
        }  // namespace slot
    }      // namespace message
}  // namespace cerberus

#endif  // CERBERUS_MESSAGE_SLOT_DOUBLESLOT_H
