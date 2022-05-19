#ifndef CERBERUS_MESSAGE_SLOT_STRINGSLOT_H
#define CERBERUS_MESSAGE_SLOT_STRINGSLOT_H

#include "baseslot.h"

namespace cerberus
{
    namespace message
    {
        namespace slot
        {
            class CERBERUS_EXPORT StringSlot : public BaseSlot
            {
                private:
                    std::string* m_string;

                public:
                    static cerberus_slot create(const std::string& string = std::string());

                    static cerberus_slot createFrom(const StringSlot& other);

                    StringSlot(const std::string& string = std::string());

                    StringSlot(const StringSlot& other);

                    virtual ~StringSlot();

                    std::string value() const;

                    void setValue(const std::string& value);
            };
        }
    }
}

#endif // CERBERUS_MESSAGE_SLOT_STRINGSLOT_H
