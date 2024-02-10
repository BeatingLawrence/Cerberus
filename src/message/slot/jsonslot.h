#ifndef CERBERUS_MESSAGE_SLOT_JSONSLOT_H
#define CERBERUS_MESSAGE_SLOT_JSONSLOT_H

#include "../../Cerberus_global.h"
#include "../../data/jsondata.h"
#include "baseslot.h"

namespace cerberus
{
    namespace message
    {
        namespace slot
        {
            class CERBERUS_EXPORT JsonSlot : public BaseSlot
            {
               private:
                data::JsonData m_value;

               public:
                static cerberus_slot create(const data::JsonData& value = data::JsonData());

                static cerberus_slot create(const JsonSlot& other);

                JsonSlot(const data::JsonData& value = data::JsonData());

                const data::JsonData& value() const;

                data::JsonData& value();

                void value(const data::JsonData& value);
            };
        }  // namespace slot
    }      // namespace message
}  // namespace cerberus

#endif  // CERBERUS_MESSAGE_SLOT_JSONSLOT_H
