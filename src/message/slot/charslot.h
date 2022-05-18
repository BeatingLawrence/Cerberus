#ifndef CERBERUS_MESSAGE_SLOT_CHARSLOT_H
#define CERBERUS_MESSAGE_SLOT_CHARSLOT_H

#include "baseslot.h"
#include "../../Cerberus_global.h"

namespace cerberus
{
    namespace message
    {
        namespace slot
        {
            class CERBERUS_EXPORT CharSlot : public BaseSlot
            {
                private:
                    char m_value;

                public:
                    static cerberus_slot create(char value = 0);

                    static cerberus_slot createFrom(const CharSlot& other);

                    CharSlot(char value = 0);

                    CharSlot(const CharSlot& other);

                    char value() const;

                    void setValue(char value);
            };
        }
    }
}

#endif // CERBERUS_MESSAGE_SLOT_CHARSLOT_H
