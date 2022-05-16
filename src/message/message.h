#ifndef CERBERUS_MESSAGE_MESSAGE_H
#define CERBERUS_MESSAGE_MESSAGE_H

#include <memory>
#include <vector>
#include "../define.h"
#include "../Cerberus_global.h"

namespace cerberus
{
    namespace message
    {
        typedef std::shared_ptr<class Message> cerberus_message;
        typedef std::shared_ptr<const class Message> cerberus_const_message;

        namespace slot
        {
            typedef std::shared_ptr<class BaseSlot> cerberus_slot;

            enum CERBERUS_EXPORT SlotType
            {
                ST_UCHAR,       //1 byte
                ST_CHAR,        //1 byte
                ST_USHORT,      //2 byte
                ST_SHORT,       //2 byte
                ST_ULONG,       //4 byte
                ST_LONG,        //4 byte
                ST_ULONGLONG,   //8 byte
                ST_LONGLONG,    //8 byte
                ST_FLOAT,       //4 byte
                ST_DOUBLE,      //8 byte
                ST_BOOL,        //1 byte
            };
        }

        class CERBERUS_EXPORT Message
        {
            private:
                std::vector<slot::cerberus_slot> m_slots;

                uint32_t m_typeID;

                uint32_t m_destinationID;

            public:
                static cerberus_message create(uint32_t typeID = CERBERUS_INVALID_ID);

                static cerberus_message createFrom(const Message& other);

                Message(uint32_t typeID = CERBERUS_INVALID_ID);

                Message(const Message& other);

                size_t count() const;

                void addSlot(slot::cerberus_slot slot);

                slot::cerberus_slot getSlotAt(size_t index) const;

                slot::cerberus_slot getSlotById(uint32_t id) const;

                uint32_t typeID() const;

                uint32_t destinationID() const;

                void setDestinationID(uint32_t id);

                bool isValid() const;
        };
    }
}

#endif // CERBERUS_MESSAGE_MESSAGE_H
