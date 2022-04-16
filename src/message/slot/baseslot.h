#ifndef CERBERUS_MESSAGE_SLOT_BASESLOT_H
#define CERBERUS_MESSAGE_SLOT_BASESLOT_H

#include <memory>
#include "../../define.h"

namespace cerberus
{
    namespace message
    {
        namespace slot
        {
            typedef std::shared_ptr<class BaseSlot> cerberus_slot;
            typedef std::shared_ptr<const class BaseSlot> cerberus_const_slot;

            class BaseSlot
            {
                private:
                    uint32_t m_id;

                public:
                    BaseSlot() = delete;

                    BaseSlot(uint32_t id = SLOT_INVALID_ID);

                    BaseSlot(const BaseSlot& other);

                    uint32_t id() const;
            };
        }
    }
}

#endif // CERBERUS_MESSAGE_SLOT_BASESLOT_H
