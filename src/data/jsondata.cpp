#include "jsondata.h"

#include "src/cerberus.h"

using namespace cerberus::data;
using namespace cerberus::core;

//=============================================================================
cerberus::OperationResult JsonData::_parse(const ByteBuffer &buffer, ParseMode mode)
{
    m_type = JDT_Null;

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
                ParseMode currentMode = None;
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
                    case None:
                        return {OR_WrongArgument, CerberusUtils::strPrint("Unexpected bracket at char %u", buffer.prev().pos())};
                    case ParsingObject:
                        return (int64_t)0;
                    case ParsingArray:
                        return {OR_WrongArgument, CerberusUtils::strPrint("Block terminated by wrong bracket at char %u", buffer.prev().pos())};
                }

            case ']':
                switch (mode)
                {
                    case None:
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
                set(res.str);
                buffer.next();  // consume the "
                buffer.consumeBlank();
            }
            break;

            case ':':
            {
                JsonData data;
                auto res = data._parse(buffer, mode);
                if (res.fail()) return res;
                m_elements.push_back(data);
                return (int64_t)res.isTrue();
            }
            break;

            case ',':
                return (int64_t)1;

            default:
            {
                ByteBuffer bb(1, b);
                bb.append(buffer.consumeUntil(" ,\"'<>?/!\n\r"));
                buffer.consumeBlank();
                auto res = bb.toNormalizedString();
                // logDebug("Captured non-string: %s", res.str.c_str());
                if (res.isTrue()) return {OR_WrongArgument, "Non textual data found"};

                if (CerberusUtils::isNumber(res.str))
                {
                    m_type = JDT_Number;
                    m_text = res.str;
                }
                else if (CerberusUtils::isBool(res.str))
                {
                    m_type = JDT_Boolean;
                    m_text = res.str;
                }
                else if (CerberusUtils::areEqual(res.str, "null", WM_CaseInsensitive))
                {
                    m_type = JDT_Null;
                    m_text = "";
                }
                else
                {
                    return {OR_WrongArgument, CerberusUtils::strPrint("Invalid token: %s", res.str.c_str())};
                }
            }
        }
    }

    return OR_OK;
}
//=============================================================================
void JsonData::_generate(ByteBuffer &buffer)
{
    update();

    buffer += toText().c_str();

    switch (m_type)
    {
        case JDT_Array:
            buffer += '[';
            break;
        case JDT_Object:
            buffer += '{';
            break;
        default:
            break;
    }

    auto counter = m_elements.size();

    for (auto &el : m_elements)
    {
        el._generate(buffer);
        if (--counter != 0) buffer += ',';
    }

    switch (m_type)
    {
        case JDT_Array:
            buffer += "]";
            break;
        case JDT_Object:
            buffer += "}";
            break;
        default:
            break;
    }
}
//=============================================================================
void JsonData::update()
{
    if (m_type != JDT_Object && m_type != JDT_Array && m_type != JDT_String) m_elements.clear();
}
//=============================================================================
std::string JsonData::toText()
{
    if (m_type == JDT_Null) return "null";

    if (m_text.empty()) return "";

    std::string ret;

    switch (m_type)
    {
        case JDT_Array:
        case JDT_Object:
        case JDT_String:
            ret = CerberusUtils::strPrint("\"%s\"", m_text.c_str());
            break;
        default:
            return CerberusUtils::strPrint("%s", m_text.c_str());
    }

    if (!m_elements.empty()) ret.append(": ");

    return ret;
}

//=============================================================================
JsonData::JsonData()
    : m_text(),
      m_elements(),
      m_type(JDT_Null)
{
}
//=============================================================================
JsonData::JsonData(const std::string &value)
    : m_text(),
      m_elements(),
      m_type(JDT_Null)
{
    set(value);
}
//=============================================================================
JsonData::JsonData(const char *value)
    : m_text(),
      m_elements(),
      m_type(JDT_Null)
{
    set(value);
}
//=============================================================================
JsonData::JsonData(long double value)
    : m_text(),
      m_elements(),
      m_type(JDT_Null)
{
    setDouble(value);
}
//=============================================================================
JsonData::JsonData(float value)
    : m_text(),
      m_elements(),
      m_type(JDT_Null)
{
    setDouble(value);
}
//=============================================================================
JsonData::JsonData(bool value)
    : m_text(),
      m_elements(),
      m_type(JDT_Null)
{
    setBool(value);
}
//=============================================================================
JsonData::JsonData(const std::string &value, const JsonData &data)
    : m_text(value),
      m_elements(1, data),
      m_type(JDT_Null)
{
}
//=============================================================================
JsonData::JsonData(const char *value, const JsonData &data)
    : m_text(value),
      m_elements(1, data),
      m_type(JDT_Null)
{
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
JsonData *JsonData::search(const std::string &name)
{
    for (auto &el : m_elements)
        if (core::CerberusUtils::areEqual(el.m_text, name)) return &el;

    return nullptr;
}
//=============================================================================
JsonData *JsonData::deepSearch(const std::string &name)
{
    JsonData *found = search(name);
    if (found) return found;

    for (auto &el : m_elements)
    {
        found = el.deepSearch(name);
        if (found) return found;
    }

    return nullptr;
}
//=============================================================================
cerberus::JsonDataType JsonData::type() { return m_type; }
//=============================================================================
bool JsonData::isNull() { return m_type == JDT_Null; }
//=============================================================================
bool JsonData::isArray() { return m_type == JDT_Array; }
//=============================================================================
bool JsonData::isObject() { return m_type == JDT_Object; }
//=============================================================================
cerberus::SIZE JsonData::size() { return m_elements.size(); }
//=============================================================================
cerberus::OperationResult JsonData::toNumber()
{
    if (m_type != JDT_Number) return OR_Unavailable;

    return OperationResult(CerberusUtils::stringToDouble(m_text));
}
//=============================================================================
cerberus::OperationResult JsonData::toString()
{
    if (m_type != JDT_String) return OR_Unavailable;

    return m_text;
}
//=============================================================================
cerberus::OperationResult JsonData::toBool()
{
    if (m_type != JDT_Boolean) return OR_Unavailable;

    if (CerberusUtils::areEqual(m_text, "true", WM_CaseInsensitive)) return (int64_t)1;

    return (int64_t)0;
}
//=============================================================================
void JsonData::setDouble(long double value)
{
    m_elements.clear();

    m_text = CerberusUtils::strPrint("%Lf", value);

    CerberusUtils::cleanNumber(m_text);

    m_type = JDT_Number;
}
//=============================================================================
void JsonData::set(const std::string &value)
{
    m_elements.clear();

    m_text = value;

    m_type = JDT_String;
}
//=============================================================================
void JsonData::setBool(bool value)
{
    m_elements.clear();

    if (value)
        m_text = "true";
    else
        m_text = "false";

    m_type = JDT_Boolean;
}
//=============================================================================
JsonData &JsonData::setType(JsonDataType type)
{
    m_type = type;
    return *this;
}
//=============================================================================
JsonData &JsonData::add(const JsonData &value)
{
    m_elements.push_back(value);
    return *this;
}
//=============================================================================
cerberus::OperationResult JsonData::parse(const ByteBuffer &buffer)
{
    m_elements.clear();
    m_text.clear();
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
void JsonData::toStr(std::string &str, uint8_t level)
{
    std::string indent(level * 2, ' ');

    str.append("\n");

    str.append(indent);

    str.append("Value: ").append(m_text).append("\n");

    str.append(indent);

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

    str.append("\n");

    str.append(indent);

    if (m_elements.empty()) return;

    str.append("Contains:\n");

    str.append(indent);

    for (auto &el : m_elements) el.toStr(str, level + 1);

    str.append("end\n");

    str.append(indent);
}
//=============================================================================
