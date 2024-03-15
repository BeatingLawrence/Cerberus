#ifndef CERBERUS_BASESLOT_H
#define CERBERUS_BASESLOT_H

#include <boost/core/demangle.hpp>

#include "../../Cerberus_global.h"
#include "../../core/cerberusutils.h"
#include "../../data/data.h"
#include "../../exception/exception.h"
#include "../../types.h"
#include "src/core/cerberuslog.h"

namespace cerberus
{
    class CERBERUS_EXPORT BaseSlot : public Clonable
    {
       private:
        std::string m_name;

       protected:
        BaseSlot(const std::string& name = std::string());

       public:
        virtual ~BaseSlot();

        // Get the type
        virtual SlotType type() const;

        // Get the name
        std::string name() const;

        // Perform a dynamic cast of this object into T.
        // An exception will be thrown if cast is invalid.
        template <class T>
        T* to()
        {
            T* casted = dynamic_cast<T*>(this);

            if (casted == nullptr)
            {
                throw cerberusIllegalArgExc(
                    core::CerberusUtils::strPrint("Invalid cast from %s to %s",
                                                  boost::core::demangle(typeid(*this).name()).c_str(),
                                                  boost::core::demangle(typeid(T).name()).c_str())
                        .c_str());
            }

            return casted;
        }
    };

    template <typename T>
    class CERBERUS_EXPORT Slot : public BaseSlot
    {
        T m_value;

       protected:
        Slot(const T& value = T(), const std::string& name = "")
            : BaseSlot(name),
              m_value(value){};

        Slot(const Slot& other) = default;

        Slot& operator=(const Slot& other) = delete;

       public:
        Slot() = delete;

        virtual ~Slot(){};

        const T& value() const { return m_value; };

        T& value() { return m_value; };

        void value(const T& value) { m_value = value; };
    };

    struct CERBERUS_EXPORT BoolSlot : public Slot<bool>
    {
        BoolSlot(bool value = false, const std::string& name = "")
            : Slot<bool>(value, name){};
        virtual SlotType type() const { return ST_BOOL; };

        static slot_ptr create(bool value = false, const std::string& name = "")
        {
            return new BoolSlot(value, name);
        };

        virtual Clonable* clone() const { return new BoolSlot(*this); };
    };

    struct CERBERUS_EXPORT BufferSlot : public Slot<data::ByteBuffer>
    {
        BufferSlot(const data::ByteBuffer& value = data::ByteBuffer(), const std::string& name = "")
            : Slot<data::ByteBuffer>(value, name){};
        virtual SlotType type() const { return ST_BYTEBUFFER; };

        static slot_ptr create(const data::ByteBuffer& value = data::ByteBuffer(),
                               const std::string& name       = "")
        {
            return new BufferSlot(value, name);
        };

        virtual Clonable* clone() const { return new BufferSlot(*this); };
    };

    struct CERBERUS_EXPORT ByteSlot : public Slot<BYTE>
    {
        ByteSlot(BYTE value = 0, const std::string& name = "")
            : Slot<BYTE>(value, name){};
        virtual SlotType type() const { return ST_BYTE; };

        static slot_ptr create(BYTE value = 0, const std::string& name = "")
        {
            return new ByteSlot(value, name);
        };

        virtual Clonable* clone() const { return new ByteSlot(*this); };
    };

    struct CERBERUS_EXPORT DictionarySlot : public Slot<Dictionary>
    {
        DictionarySlot(Dictionary value = Dictionary(), const std::string& name = "")
            : Slot<Dictionary>(value, name){};
        virtual SlotType type() const { return ST_DICTIONARY; };

        static slot_ptr create(const Dictionary& value = Dictionary(), const std::string& name = "")
        {
            return new DictionarySlot(value, name);
        };

        virtual Clonable* clone() const { return new DictionarySlot(*this); };
    };

    struct CERBERUS_EXPORT DoubleSlot : public Slot<double>
    {
        DoubleSlot(double value = 0.0f, const std::string& name = "")
            : Slot<double>(value, name){};
        virtual SlotType type() const { return ST_DOUBLE; };

        static slot_ptr create(double value = 0.0f, const std::string& name = "")
        {
            return new DoubleSlot(value, name);
        };

        virtual Clonable* clone() const { return new DoubleSlot(*this); };
    };

    struct CERBERUS_EXPORT FloatSlot : public Slot<float>
    {
        FloatSlot(float value = 0.0f, const std::string& name = "")
            : Slot<float>(value, name){};
        virtual SlotType type() const { return ST_FLOAT; };

        static slot_ptr create(float value = 0.0f, const std::string& name = "")
        {
            return new FloatSlot(value, name);
        };

        virtual Clonable* clone() const { return new FloatSlot(*this); };
    };

    struct CERBERUS_EXPORT Int32Slot : public Slot<int32_t>
    {
        Int32Slot(int32_t value = 0, const std::string& name = "")
            : Slot<int32_t>(value, name){};
        virtual SlotType type() const { return ST_INT32; };

        static slot_ptr create(int32_t value = 0, const std::string& name = "")
        {
            return new Int32Slot(value, name);
        };

        virtual Clonable* clone() const { return new Int32Slot(*this); };
    };

    struct CERBERUS_EXPORT Int64Slot : public Slot<int64_t>
    {
        Int64Slot(int64_t value = 0, const std::string& name = "")
            : Slot<int64_t>(value, name){};
        virtual SlotType type() const { return ST_INT64; };

        static slot_ptr create(int64_t value = 0, const std::string& name = "")
        {
            return new Int64Slot(value, name);
        };

        virtual Clonable* clone() const { return new Int64Slot(*this); };
    };

    struct CERBERUS_EXPORT JsonSlot : public Slot<data::JsonData>
    {
        JsonSlot(data::JsonData value = data::JsonData(), const std::string& name = "")
            : Slot<data::JsonData>(value, name){};
        virtual SlotType type() const { return ST_JSON; };

        static slot_ptr create(const data::JsonData& value = data::JsonData(), const std::string& name = "")
        {
            return new JsonSlot(value, name);
        };

        virtual Clonable* clone() const { return new JsonSlot(*this); };
    };

    struct CERBERUS_EXPORT StringSlot : public Slot<std::string>
    {
        StringSlot(std::string value = std::string(), const std::string& name = "")
            : Slot<std::string>(value, name){};
        virtual SlotType type() const { return ST_STRING; };

        static slot_ptr create(const std::string& value = std::string(), const std::string& name = "")
        {
            return new StringSlot(value, name);
        };

        virtual Clonable* clone() const { return new StringSlot(*this); };
    };

    struct CERBERUS_EXPORT VoidPSlot : public Slot<void*>
    {
        VoidPSlot(void* value = nullptr, const std::string& name = "")
            : Slot<void*>(value, name){};
        virtual SlotType type() const { return ST_VOIDP; };

        static slot_ptr create(void* value = nullptr, const std::string& name = "")
        {
            return new VoidPSlot(value, name);
        };

        virtual Clonable* clone() const { return new VoidPSlot(*this); };
    };

}  // namespace cerberus

#endif  // CERBERUS_BASESLOT_H
