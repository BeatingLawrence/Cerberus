#ifndef CERBERUS_BASESLOT_H
#define CERBERUS_BASESLOT_H

#include <boost/core/demangle.hpp>

#include "../Cerberus_global.h"
#include "../core/cerberusutils.h"
#include "../data/data.h"
#include "../exception/exception.h"
#include "../types.h"

namespace cerberus
{
    class CERBERUS_EXPORT SlotBase : public Clonable
    {
       private:
        HASH32 m_id;

       protected:
        SlotBase(const std::string& name = std::string());
        SlotBase(HASH32 id);

       public:
        virtual ~SlotBase();

        // Set the ID, recomputing hash
        SlotBase& setId(const std::string& name);
        SlotBase& setId(HASH32 id);

        HASH32 id() const;

        virtual Clonable* clone() const = 0;

        virtual ByteBuffer toBuffer() const = 0;

        virtual slot_ptr newslot() const = 0;  // allocates T, used for factory

        // Perform a dynamic cast of this object into T.
        // An exception will be thrown if cast is invalid.
        template <class T>
        T* to()
        {
            T* casted = dynamic_cast<T*>(this);

            if (casted == nullptr)
            {
                throw cIllegalArgExc(
                    CerberusUtils::strPrint("Invalid cast from %s to %s",
                                            boost::core::demangle(typeid(*this).name()).c_str(),
                                            boost::core::demangle(typeid(T).name()).c_str())
                        .c_str());
            }

            return casted;
        }
    };

    template <typename T>
    class CERBERUS_EXPORT Slot : public SlotBase
    {
       protected:
        T m_value;

        Slot(const T& value, const std::string& name = "")
            : SlotBase(name),
              m_value(value) {};

        Slot(const Slot& other) = default;

        Slot() = delete;

        Slot& operator=(const Slot& other) = delete;

       public:
        virtual ~Slot() {}

        const T& value() const { return m_value; }

        T& value() { return m_value; }

        void value(const T& value) { m_value = value; }
    };

    // This macro allows the application to define a new slot type using a given C-struct
#define new_slot_from_cstruct(slot_type, c_struct)                                                 \
    struct slot_type : public cerberus::Slot<c_struct>                                             \
    {                                                                                              \
        slot_type(const c_struct& value = {}, const std::string& name = "")                        \
            : cerberus::Slot<c_struct>(value, name) {};                                            \
        static cerberus::slot_ptr create(const c_struct& value = {}, const std::string& name = "") \
        {                                                                                          \
            return cerberus::slot_ptr(new slot_type(value, name));                                 \
        }                                                                                          \
        virtual cerberus::slot_ptr newslot() const { return create(); }                            \
        virtual cerberus::Clonable* clone() const { return new slot_type(*this); }                 \
        virtual cerberus::ByteBuffer toBuffer() const                                              \
        {                                                                                          \
            return cerberus::ByteBuffer((cerberus::BYTE*)&m_value, sizeof(c_struct));              \
        }                                                                                          \
        virtual cerberus::SIZE memfp() const { return sizeof(slot_type); }                         \
    };

    //====================================================================================

    struct CERBERUS_EXPORT BoolSlot : public Slot<bool>
    {
        BoolSlot(bool value = false, const std::string& name = "")
            : Slot<bool>(value, name) {};

        static slot_ptr create(bool value = false, const std::string& name = "")
        {
            return slot_ptr(new BoolSlot(value, name));
        }

        virtual slot_ptr newslot() const { return create(); }

        virtual Clonable* clone() const { return new BoolSlot(*this); }

        virtual ByteBuffer toBuffer() const { return ByteBuffer((BYTE*)&m_value, sizeof(bool)); }

        virtual SIZE memfp() const { return sizeof(BoolSlot); }
    };

    struct CERBERUS_EXPORT BufferSlot : public Slot<ByteBuffer>
    {
        BufferSlot(const ByteBuffer& value = ByteBuffer(), const std::string& name = "")
            : Slot<ByteBuffer>(value, name) {};

        static slot_ptr create(const ByteBuffer& value = ByteBuffer(), const std::string& name = "")
        {
            return slot_ptr(new BufferSlot(value, name));
        }

        virtual slot_ptr newslot() const { return create(); }

        virtual Clonable* clone() const { return new BufferSlot(*this); }

        virtual ByteBuffer toBuffer() const { return m_value; }

        virtual SIZE memfp() const { return sizeof(BufferSlot); }
    };

    struct CERBERUS_EXPORT ByteSlot : public Slot<BYTE>
    {
        ByteSlot(BYTE value = 0, const std::string& name = "")
            : Slot<BYTE>(value, name) {};

        static slot_ptr create(BYTE value = 0, const std::string& name = "")
        {
            return slot_ptr(new ByteSlot(value, name));
        }

        virtual slot_ptr newslot() const { return create(); }

        virtual Clonable* clone() const { return new ByteSlot(*this); }

        virtual ByteBuffer toBuffer() const { return ByteBuffer((BYTE*)&m_value, sizeof(BYTE)); }

        virtual SIZE memfp() const { return sizeof(ByteSlot); }
    };

    struct CERBERUS_EXPORT DictionarySlot : public Slot<Dictionary>
    {
        DictionarySlot(Dictionary value = Dictionary(), const std::string& name = "")
            : Slot<Dictionary>(value, name) {};

        static slot_ptr create(const Dictionary& value = Dictionary(), const std::string& name = "")
        {
            return slot_ptr(new DictionarySlot(value, name));
        }

        virtual slot_ptr newslot() const { return create(); }

        virtual Clonable* clone() const { return new DictionarySlot(*this); }

        virtual ByteBuffer toBuffer() const { return ByteBuffer(m_value.toString()); }

        virtual SIZE memfp() const { return sizeof(DictionarySlot) + m_value.memfp(); }
    };

    struct CERBERUS_EXPORT DoubleSlot : public Slot<double>
    {
        DoubleSlot(double value = 0.0f, const std::string& name = "")
            : Slot<double>(value, name) {};

        static slot_ptr create(double value = 0.0f, const std::string& name = "")
        {
            return slot_ptr(new DoubleSlot(value, name));
        }

        virtual slot_ptr newslot() const { return create(); }

        virtual Clonable* clone() const { return new DoubleSlot(*this); }

        virtual ByteBuffer toBuffer() const { return ByteBuffer((BYTE*)&m_value, sizeof(double)); }

        virtual SIZE memfp() const { return sizeof(DoubleSlot); }
    };

    struct CERBERUS_EXPORT FloatSlot : public Slot<float>
    {
        FloatSlot(float value = 0.0f, const std::string& name = "")
            : Slot<float>(value, name) {};

        static slot_ptr create(float value = 0.0f, const std::string& name = "")
        {
            return slot_ptr(new FloatSlot(value, name));
        }

        virtual slot_ptr newslot() const { return create(); }

        virtual Clonable* clone() const { return new FloatSlot(*this); }

        virtual ByteBuffer toBuffer() const { return ByteBuffer((BYTE*)&m_value, sizeof(float)); }

        virtual SIZE memfp() const { return sizeof(FloatSlot); }
    };

    struct CERBERUS_EXPORT Int32Slot : public Slot<int32_t>
    {
        Int32Slot(int32_t value = 0, const std::string& name = "")
            : Slot<int32_t>(value, name) {};

        static slot_ptr create(int32_t value = 0, const std::string& name = "")
        {
            return slot_ptr(new Int32Slot(value, name));
        }

        virtual slot_ptr newslot() const { return create(); }

        virtual Clonable* clone() const { return new Int32Slot(*this); }

        virtual ByteBuffer toBuffer() const { return ByteBuffer((BYTE*)&m_value, sizeof(int32_t)); }

        virtual SIZE memfp() const { return sizeof(Int32Slot); }
    };

    struct CERBERUS_EXPORT Int64Slot : public Slot<int64_t>
    {
        Int64Slot(int64_t value = 0, const std::string& name = "")
            : Slot<int64_t>(value, name) {};

        static slot_ptr create(int64_t value = 0, const std::string& name = "")
        {
            return slot_ptr(new Int64Slot(value, name));
        }

        virtual slot_ptr newslot() const { return create(); }

        virtual Clonable* clone() const { return new Int64Slot(*this); }

        virtual ByteBuffer toBuffer() const { return ByteBuffer((BYTE*)&m_value, sizeof(int64_t)); }

        virtual SIZE memfp() const { return sizeof(Int64Slot); }
    };

    struct CERBERUS_EXPORT UInt64Slot : public Slot<uint64_t>
    {
        UInt64Slot(uint64_t value = 0, const std::string& name = "")
            : Slot<uint64_t>(value, name) {};

        static slot_ptr create(uint64_t value = 0, const std::string& name = "")
        {
            return slot_ptr(new UInt64Slot(value, name));
        }

        virtual slot_ptr newslot() const { return create(); }

        virtual Clonable* clone() const { return new UInt64Slot(*this); }

        virtual ByteBuffer toBuffer() const { return ByteBuffer((BYTE*)&m_value, sizeof(uint64_t)); }

        virtual SIZE memfp() const { return sizeof(UInt64Slot); }
    };

    struct CERBERUS_EXPORT JsonSlot : public Slot<JsonData>
    {
        JsonSlot(JsonData value = JsonData(), const std::string& name = "")
            : Slot<JsonData>(value, name) {};

        static slot_ptr create(const JsonData& value = JsonData(), const std::string& name = "")
        {
            return slot_ptr(new JsonSlot(value, name));
        }

        virtual slot_ptr newslot() const { return create(); }

        virtual Clonable* clone() const { return new JsonSlot(*this); }

        virtual ByteBuffer toBuffer() const
        {
            return m_value.generate().value;
        }  // throw an exception if conversion fails?

        virtual SIZE memfp() const { return sizeof(JsonSlot) + m_value.memfp(); }
    };

    struct CERBERUS_EXPORT StringSlot : public Slot<std::string>
    {
        StringSlot(std::string value = std::string(), const std::string& name = "")
            : Slot<std::string>(value, name) {};

        static slot_ptr create(const std::string& value = std::string(), const std::string& name = "")
        {
            return slot_ptr(new StringSlot(value, name));
        }

        virtual slot_ptr newslot() const { return create(); }

        virtual Clonable* clone() const { return new StringSlot(*this); }

        virtual ByteBuffer toBuffer() const { return ByteBuffer(m_value); }

        virtual SIZE memfp() const { return sizeof(StringSlot) + m_value.capacity(); }
    };

    struct CERBERUS_EXPORT VoidPSlot : public Slot<void*>
    {
        VoidPSlot(void* value = nullptr, const std::string& name = "")
            : Slot<void*>(value, name) {};

        static slot_ptr create(void* value = nullptr, const std::string& name = "")
        {
            return slot_ptr(new VoidPSlot(value, name));
        }

        virtual slot_ptr newslot() const { return create(); }

        virtual Clonable* clone() const { return new VoidPSlot(*this); }

        virtual ByteBuffer toBuffer() const { throw cUsageErrorExc("void* is not convertible to a buffer"); }

        virtual SIZE memfp() const { return sizeof(VoidPSlot); }
    };

    struct CERBERUS_EXPORT HostSlot : public Slot<Host>
    {
        HostSlot(const Host& value = Host(), const std::string& name = "")
            : Slot<Host>(value, name) {};

        static slot_ptr create(const Host& value = Host(), const std::string& name = "")
        {
            return slot_ptr(new HostSlot(value, name));
        }

        virtual slot_ptr newslot() const { return create(); }

        virtual Clonable* clone() const { return new HostSlot(*this); }

        virtual ByteBuffer toBuffer() const { return ByteBuffer(m_value.toString()); }

        virtual SIZE memfp() const { return sizeof(HostSlot) + m_value.hostname.capacity(); }
    };

    struct CERBERUS_EXPORT TaskSlot : public Slot<Task>
    {
        TaskSlot(Task value = {}, const std::string& name = "")
            : Slot<Task>(value, name) {};

        static slot_ptr create(Task value = {}, const std::string& name = "")
        {
            return slot_ptr(new TaskSlot(value, name));
        }

        virtual slot_ptr newslot() const { return create(); }

        virtual Clonable* clone() const { return new TaskSlot(*this); }

        virtual ByteBuffer toBuffer() const { throw cUsageErrorExc("Task is not convertible to a buffer"); }

        virtual SIZE memfp() const { return sizeof(TaskSlot); }
    };

    struct CERBERUS_EXPORT ResultSlot : public Slot<OpRes>
    {
        ResultSlot(OpRes value = OpRes(), const std::string& name = "")
            : Slot<OpRes>(value, name) {};

        static slot_ptr create(OpRes value = OpRes(), const std::string& name = "")
        {
            return slot_ptr(new ResultSlot(value, name));
        }

        virtual slot_ptr newslot() const { return create(); }

        virtual Clonable* clone() const { return new ResultSlot(*this); }

        virtual ByteBuffer toBuffer() const { return ByteBuffer(m_value.toStr()); }

        virtual SIZE memfp() const { return sizeof(ResultSlot) + m_value.memfp(); }
    };

}  // namespace cerberus

#endif  // CERBERUS_BASESLOT_H
