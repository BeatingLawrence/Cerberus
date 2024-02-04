#ifndef CERBERUS_MESSAGE_SLOT_BYTEBUFFERSLOT_H
#define CERBERUS_MESSAGE_SLOT_BYTEBUFFERSLOT_H

#include "../../Cerberus_global.h"
#include "../../data/bytebuffer.h"
#include "baseslot.h"

namespace cerberus
{
    namespace message
    {
        namespace slot
        {
            class CERBERUS_EXPORT ByteBufferSlot : public BaseSlot
            {
               private:
                data::ByteBuffer m_value;

               public:
                static cerberus_slot create(const data::ByteBuffer& value = data::ByteBuffer());

                static cerberus_slot create(const ByteBufferSlot& other);

                ByteBufferSlot(const data::ByteBuffer& value = data::ByteBuffer());

                const data::ByteBuffer& value() const;

                data::ByteBuffer& value();

                void value(const data::ByteBuffer& value);
            };
        }  // namespace slot
    }      // namespace message
}  // namespace cerberus

#endif  // CERBERUS_MESSAGE_SLOT_BYTEBUFFERSLOT_H
