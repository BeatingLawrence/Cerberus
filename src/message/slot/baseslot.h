#ifndef CERBERUS_MESSAGE_SLOT_BASESLOT_H
#define CERBERUS_MESSAGE_SLOT_BASESLOT_H

/*  This is the Slots base class.
 *  This class cannot be instantiated, but it must be extended by a derived class which contains the data itself.
 *
 *  Data type is specified in the SlotType enumeration
 *
 *  The ID of the Slot is chosen by the user while creating a message, and it has no effect on Cerberus dispatching and message management.
 */

#include <memory>
#include "../../define.h"
#include "../../Cerberus_global.h"

namespace cerberus
{
    namespace message
    {
        namespace slot
        {
            typedef std::shared_ptr<class BaseSlot> cerberus_slot;

            class CERBERUS_EXPORT BaseSlot
            {
                public:
                    enum SlotType
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

                private:
                    SlotType m_dataType;

                    uint32_t m_id;

                protected:
                    //Constructs a BaseSlot with a given type
                    BaseSlot(SlotType type);

                    //Copy-constructs another BaseSlot
                    BaseSlot(const BaseSlot& other);

                public:
                    BaseSlot() = delete;

                    //Gets the type
                    SlotType type() const;

                    //Gets the ID, useful for dispatching
                    uint32_t id() const;

                    //Sets an ID
                    void setId(uint32_t id);

                    //Performs a dynamic cast of this object into T. An exception will be thrown if cast is invalid.
                    template<class T> T* to();

                    //Performs a dynamic cast of from into a shared_ptr of type T, caring for the instance counter.
                    //An exception will be thrown if cast is invalid.
                    template<class T> static std::shared_ptr<T> toShared(const cerberus_slot& from);
            };
        }
    }
}

#endif // CERBERUS_MESSAGE_SLOT_BASESLOT_H
