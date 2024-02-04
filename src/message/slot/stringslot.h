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
                std::string m_value;

               public:
                static cerberus_slot create(const std::string& string = std::string());

                static cerberus_slot create(const StringSlot& other);

                StringSlot(const std::string& string = std::string());

                std::string value() const;

                void value(const std::string& value);
            };
        }  // namespace slot
    }      // namespace message
}  // namespace cerberus

#endif  // CERBERUS_MESSAGE_SLOT_STRINGSLOT_H
