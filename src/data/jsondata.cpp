#include "jsondata.h"

#include "src/cerberus.h"

using namespace cerberus::data;
using namespace cerberus::core;

//=============================================================================
cerberus::OperationResult JsonData::_parse(const ByteBuffer &buffer, ParseMode mode)
{
    m_type     = JDT_Null;
    bool value = false;

    while (!buffer.isEnd())
    {
        if (buffer.consumeBlank().isEnd()) break;

        auto b = buffer.readByte();
        // logDebug("PARSED %c", (char)b);

        switch (b)
        {
            case '{':
            case '[':
            {
                ParseMode currentMode = Unspecified;
                if (b == '{')
                {
                    m_type      = JDT_Object;
                    currentMode = ParsingObject;
                }
                else
                {
                    m_type      = JDT_Array;
                    currentMode = ParsingArray;
                }

                // process the object
                bool flag = true;
                while (flag)
                {
                    JsonData data;
                    auto res = data._parse(buffer, currentMode);
                    if (res.fail()) return res;
                    flag = res.isTrue();
                    m_elements.push_back(data);
                }
            }
            break;

            case '}':
                switch (mode)
                {
                    case Unspecified:
                        return {OR_WrongArgument, CerberusUtils::strPrint("Unexpected bracket at char %u", buffer.prev().pos())};
                    case ParsingObject:
                        return (int64_t)0;
                    case ParsingArray:
                        return {OR_WrongArgument, CerberusUtils::strPrint("Block terminated by wrong bracket at char %u", buffer.prev().pos())};
                }

            case ']':
                switch (mode)
                {
                    case Unspecified:
                        return {OR_WrongArgument, CerberusUtils::strPrint("Unexpected bracket at char %u", buffer.prev().pos())};
                    case ParsingObject:
                        return {OR_WrongArgument, CerberusUtils::strPrint("Block terminated by wrong bracket at char %u", buffer.prev().pos())};
                    case ParsingArray:
                        return (int64_t)0;
                }

            case '"':
            {
                // start of a string
                auto res = buffer.consumeUntil("\"").toNormalizedString();
                // logDebug("Captured: %s", res.str.c_str());
                if (res.isTrue()) return {OR_WrongArgument, "Non textual data found"};
                buffer.next();  // consume the "

                if (mode == ParsingArray)
                {
                    setString(res.str);
                }
                else
                {
                    if (value)
                        setString(res.str);
                    else
                        m_name = res.str;
                }
            }
            break;

            case ':':
            {
                if (value || mode == ParsingArray) return {OR_WrongArgument, CerberusUtils::strPrint("Unexpected column at char %u", buffer.prev().pos())};
                value = true;
            }
            break;

            case ',':
                return (int64_t)1;

            default:
            {
                ByteBuffer bb(1, b);
                bb.append(buffer.consumeUntil(" ,[]{}\n\r\"'<>?/!"));
                auto res = bb.toNormalizedString();
                // logDebug("Captured non-string: %s", res.str.c_str());
                if (res.isTrue()) return {OR_WrongArgument, "Non textual data found"};
                if (!value && mode == ParsingObject) return {OR_WrongArgument, CerberusUtils::strPrint("No column before value at char %u", buffer.pos())};

                if (CerberusUtils::isNumber(res.str))
                {
                    m_type  = JDT_Number;
                    m_value = res.str;
                }
                else if (CerberusUtils::isBool(res.str))
                {
                    m_type  = JDT_Boolean;
                    m_value = res.str;
                }
                else if (CerberusUtils::areEqual(res.str, "null", WM_CaseInsensitive))
                {
                    m_type = JDT_Null;
                    m_value.clear();
                }
                else
                {
                    return {OR_WrongArgument, CerberusUtils::strPrint("Invalid token: '%s' at %u", res.str.c_str(), buffer.pos())};
                }
            }
        }
    }

    return OR_OK;
}
//=============================================================================
void JsonData::_generate(ByteBuffer &buffer)
{
    if (!m_name.empty())
    {
        buffer += '"';
        buffer += m_name;
        buffer += "\": ";
    }

    switch (m_type)
    {
        case JDT_Null:
            buffer += "null";
            return;
        case JDT_Number:
        case JDT_Boolean:
            buffer += CerberusUtils::strPrint("%s", m_value.c_str());
            return;
        case JDT_String:
            buffer += CerberusUtils::strPrint("\"%s\"", m_value.c_str());
            return;
        case JDT_Array:
            buffer += "[";
            break;
        case JDT_Object:
            buffer += "{";
            break;
    }

    auto counter = m_elements.size();

    for (auto &el : m_elements)
    {
        el._generate(buffer);
        if (--counter != 0) buffer += ',';
    }

    if (m_type == JDT_Array)
        buffer += ']';
    else
        buffer += '}';
}
//=============================================================================
JsonData::Integrity JsonData::checkIntegrity() const
{
    if (m_elements.empty()) return None;

    Integrity integrity = m_elements.front().m_name.empty() ? GoodArray : GoodObject;

    for (auto &el : m_elements)
    {
        if (el.m_name.empty())
        {
            if (integrity == GoodObject) return Degraded;
        }
        else if (integrity == GoodArray)
            return Degraded;
    }

    return integrity;
}
//=============================================================================
JsonData::JsonData()
    : m_name(),
      m_value(),
      m_elements(),
      m_type(JDT_Null)
{
}
//=============================================================================
JsonData::JsonData(const std::string &name)
    : m_name(name),
      m_value(),
      m_elements(),
      m_type(JDT_Null)
{
}
//=============================================================================
JsonData::JsonData(const std::string &name, const std::string &value)
    : m_name(name),
      m_value(value),
      m_elements(),
      m_type(JDT_String)
{
}
//=============================================================================
JsonData::JsonData(const std::string &name, const char *value)
    : m_name(name),
      m_value(value),
      m_elements(),
      m_type(JDT_String)
{
}
//=============================================================================
JsonData::JsonData(const std::string &name, long double value)
    : m_name(name),
      m_value(),
      m_elements(),
      m_type(JDT_Number)
{
    setNumber(value);
}
//=============================================================================
JsonData::JsonData(const std::string &name, float value)
    : m_name(name),
      m_value(),
      m_elements(),
      m_type(JDT_Number)
{
    setNumber(value);
}
//=============================================================================
JsonData::JsonData(const std::string &name, int value)
    : m_name(name),
      m_value(),
      m_elements(),
      m_type(JDT_Number)
{
    setNumber(value);
}
//=============================================================================
JsonData::JsonData(const std::string &name, bool value)
    : m_name(name),
      m_value(),
      m_elements(),
      m_type(JDT_Boolean)
{
    setBoolean(value);
}
//=============================================================================
JsonData::JsonData(const std::string &name, const JsonData &data)
    : m_name(name),
      m_value(),
      m_elements(1, data),
      m_type(JDT_Object)
{
    if (isArray()) m_type = JDT_Array;
}
//=============================================================================
cerberus::Iterator<JsonData> JsonData::begin()
{
    if (m_elements.empty()) return nullptr;
    return &m_elements.front();
}
//=============================================================================
cerberus::Iterator<JsonData> JsonData::end()
{
    if (m_elements.empty()) return nullptr;
    return ((&m_elements.back()) + 1);
}
//=============================================================================
cerberus::ConstIterator<JsonData> JsonData::begin() const
{
    if (m_elements.empty()) return nullptr;
    return &m_elements.front();
}
//=============================================================================
cerberus::ConstIterator<JsonData> JsonData::end() const
{
    if (m_elements.empty()) return nullptr;
    return ((&m_elements.back()) + 1);
}
//=============================================================================
JsonData &JsonData::get(SIZE index)
{
    if (index >= size()) throw cerberusIllegalArgExc("index out of bounds");
    return m_elements[index];
}
//=============================================================================
JsonData JsonData::search(const std::string &name)
{
    for (auto &el : m_elements)
        if (core::CerberusUtils::areEqual(el.m_name, name)) return el;

    return JsonData();
}
//=============================================================================
JsonData JsonData::deepSearch(const std::string &name)
{
    if (isNull()) return JsonData();

    auto found = search(name);
    if (!found.isNull()) return found;

    for (auto &el : m_elements)
    {
        found = el.deepSearch(name);
        if (!found.isNull()) return found;
    }

    return JsonData();
}
//=============================================================================
cerberus::JsonDataType JsonData::type() const { return m_type; }
//=============================================================================
bool JsonData::isNull() const { return m_type == JDT_Null; }
//=============================================================================
bool JsonData::isArray() const { return m_type == JDT_Array; }
//=============================================================================
bool JsonData::isObject() const { return m_type == JDT_Object; }
//=============================================================================
cerberus::SIZE JsonData::size() const { return m_elements.size(); }
//=============================================================================
bool JsonData::empty() const { return m_elements.empty(); }
//=============================================================================
bool JsonData::check() const
{
    if (m_type == JDT_Array || m_type == JDT_Object)
    {
        auto integrity = checkIntegrity();
        if (integrity == Degraded) return false;
        if (integrity == GoodArray && m_type != JDT_Array) return false;
        if (integrity == GoodObject && m_type != JDT_Object) return false;
    }

    for (auto &el : m_elements)
        if (!el.check()) return false;

    return true;
}
//=============================================================================
cerberus::OperationResult JsonData::checkFix()
{
    if (m_type == JDT_Array || m_type == JDT_Object)
    {
        auto integrity = checkIntegrity();
        if (integrity == Degraded) return OR_WrongArgument;

        if (integrity == GoodArray)
            m_type = JDT_Array;
        else if (integrity == GoodObject)
            m_type = JDT_Object;
    }

    for (auto &el : m_elements)
    {
        auto res = el.checkFix();
        if (res.fail()) return res;
    }

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult JsonData::toNumber() const
{
    if (m_type != JDT_Number) return OR_Unavailable;
    return CerberusUtils::stringToDouble(m_value);
}
//=============================================================================
cerberus::OperationResult JsonData::toIntNumber() const
{
    if (m_type != JDT_Number) return OR_Unavailable;
    return CerberusUtils::stringToInt(m_value);
}
//=============================================================================
cerberus::OperationResult JsonData::toString() const
{
    if (m_type != JDT_String) return OR_Unavailable;
    return m_value;
}
//=============================================================================
cerberus::OperationResult JsonData::toBool() const
{
    if (m_type != JDT_Boolean) return OR_Unavailable;
    if (CerberusUtils::areEqual(m_value, "true", WM_CaseInsensitive)) return (int64_t)1;
    return (int64_t)0;
}
//=============================================================================
JsonData &JsonData::setName(const std::string &name)
{
    m_name = name;
    return *this;
}
//=============================================================================
JsonData &JsonData::setNumber(long double value)
{
    m_elements.clear();
    m_value = CerberusUtils::strPrint("%Lf", value);
    CerberusUtils::cleanNumber(m_value);
    m_type = JDT_Number;
    return *this;
}
//=============================================================================
JsonData &JsonData::setNumber(float value)
{
    m_elements.clear();
    m_value = CerberusUtils::strPrint("%f", value);
    CerberusUtils::cleanNumber(m_value);
    m_type = JDT_Number;
    return *this;
}
//=============================================================================
JsonData &JsonData::setNumber(int value)
{
    m_elements.clear();
    m_value = CerberusUtils::strPrint("%i", value);
    CerberusUtils::cleanNumber(m_value);
    m_type = JDT_Number;
    return *this;
}
//=============================================================================
JsonData &JsonData::setString(const std::string &value)
{
    m_elements.clear();
    m_value = value;
    m_type  = JDT_String;
    return *this;
}
//=============================================================================
JsonData &JsonData::setBoolean(bool value)
{
    m_elements.clear();

    if (value)
        m_value = "true";
    else
        m_value = "false";

    m_type = JDT_Boolean;

    return *this;
}
//=============================================================================
JsonData &JsonData::add(const JsonData &object)
{
    m_elements.push_back(object);

    switch (m_type)
    {
        case JDT_Null:
        case JDT_Number:
        case JDT_String:
        case JDT_Boolean:
            m_value.clear();
            if (object.m_name.empty())
                m_type = JDT_Array;
            else
                m_type = JDT_Object;
            break;
        case JDT_Array:
            if (!object.m_name.empty()) m_type = JDT_Object;
            break;
        case JDT_Object:
            break;
    }

    return *this;
}
//=============================================================================
JsonData &JsonData::add(const std::string &value) { return add(JsonData("", value)); }
//=============================================================================
JsonData &JsonData::add(const char *value) { return add(JsonData("", value)); }
//=============================================================================
JsonData &JsonData::add(long double value) { return add(JsonData("", value)); }
//=============================================================================
JsonData &JsonData::add(float value) { return add(JsonData("", value)); }
//=============================================================================
JsonData &JsonData::add(int value) { return add(JsonData("", value)); }
//=============================================================================
JsonData &JsonData::add(bool value) { return add(JsonData("", value)); }
//=============================================================================
JsonData &JsonData::clear()
{
    m_elements.clear();
    return *this;
}
//=============================================================================
cerberus::OperationResult JsonData::parse(const ByteBuffer &buffer)
{
    m_elements.clear();
    m_name.clear();
    m_value.clear();
    buffer.resetCursor();

    auto res = _parse(buffer);
    return res;
}
//=============================================================================
cerberus::OperationResult JsonData::parse(const filesystem::File &file)
{
    ByteBuffer buffer;
    auto res = file.read(buffer);

    if (res.fail()) return res;

    return parse(buffer);
}
//=============================================================================
cerberus::OperationResult JsonData::generate(ByteBuffer &buffer)
{
    buffer.clear();
    if (m_elements.empty()) return OR_Empty;
    _generate(buffer);

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult JsonData::generate(filesystem::File &file)
{
    ByteBuffer buffer;
    auto res = generate(buffer);
    if (res.fail()) return res;

    return file.write(buffer);
}
//=============================================================================
void JsonData::toStr(std::string &str, uint8_t level) const
{
    std::string indent(level * 2, ' ');

    str.append("\n").append(indent);

    switch (m_type)
    {
        case JDT_Null:
            str.append("Type: null");
            break;
        case JDT_Number:
            str.append("Type: number");
            break;
        case JDT_String:
            str.append("Type: string");
            break;
        case JDT_Boolean:
            str.append("Type: boolean");
            break;
        case JDT_Array:
            str.append("Type: array");
            break;
        case JDT_Object:
            str.append("Type: object");
            break;
    }

    if (!m_name.empty()) str.append(", Name: ").append(m_name);

    if (!m_value.empty()) str.append(", Value: ").append(m_value);

    str.append("\n").append(indent);

    if (m_elements.empty()) return;

    str.append("Contains:\n").append(indent);

    for (auto &el : m_elements) el.toStr(str, level + 1);

    str.pop_back();  // back one level
    str.pop_back();

    str.append("end\n").append(indent);
}
//=============================================================================
