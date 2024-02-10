#ifndef CERBERUS_MESSAGE_SLOT_DICTIONARY_H
#define CERBERUS_MESSAGE_SLOT_DICTIONARY_H

#include "../../Cerberus_global.h"
#include "baseslot.h"

namespace cerberus
{
    namespace message
    {
        namespace slot
        {
            class CERBERUS_EXPORT DictionarySlot : public BaseSlot
            {
               private:
                Dictionary m_dict;

               public:
                static cerberus_slot create(const Dictionary& dict = Dictionary());

                static cerberus_slot create(const DictionarySlot& other);

                DictionarySlot(const Dictionary& dict = Dictionary());

                const Dictionary& value() const;

                Dictionary& value();

                void value(const Dictionary& dict);
            };
        }  // namespace slot
    }      // namespace message
}  // namespace cerberus

#endif  // CERBERUS_MESSAGE_SLOT_DICTIONARY_H
