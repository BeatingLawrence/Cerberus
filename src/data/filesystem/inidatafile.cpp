#include "inidatafile.h"

#include "../../core/cerberusutils.h"
#include "../../types.h"

using namespace cerberus::data::filesystem;

//=============================================================================
bool IniDataFile::_isValid(const std::string& line) { return std::regex_match(line, m_isValidRegex); }
//=============================================================================
bool IniDataFile::_isInteger(const std::string& line) { return std::regex_match(line, m_isIntegerRegex); }
//=============================================================================
bool IniDataFile::_isDouble(const std::string& line) { return std::regex_match(line, m_isDoubleRegex); }
//=============================================================================
bool IniDataFile::_isBool(const std::string& line) { return std::regex_match(line, m_isBoolRegex); }
//=============================================================================
std::string IniDataFile::_getKey(const std::string& line)
{
    size_t pos = line.find_first_of('=');

    while (line[--pos] == ' ')
    {
    }

    return line.substr(0, ++pos);
}
//=============================================================================
std::string IniDataFile::_getValue(const std::string& line)
{
    size_t pos = line.find_first_of('=');

    while (line[++pos] == ' ')
    {
    }

    return line.substr(pos, std::string::npos);
}
//=============================================================================
IniDataFile::Entry* IniDataFile::_search(const std::string& key)
{
    for (auto& el : m_entries)
    {
        if (el.key.compare(key) == 0)
        {
            return &el;
        }
    }

    return nullptr;
}
//=============================================================================
bool IniDataFile::_syncFile()
{
    m_file.setOpenMode(FOM_ReadWriteTrunc);

    if (!m_file.open())
    {
        return false;
    }

    for (auto& el : m_entries)
    {
        std::string line = core::CerberusUtils::strPrint("%s = %s", el.key.c_str(), el.stringValue.c_str());
        m_file.writeLine(line);
    }

    m_file.close();

    return true;
}
//=============================================================================
IniDataFile::IniDataFile(const std::string& fileName)
    : m_file(fileName),
      m_isValidRegex("[a-z][^=]*[=]{1} *[^=]+", std::regex_constants::ECMAScript | std::regex_constants::optimize | std::regex_constants::icase),
      m_isIntegerRegex("[a-z][^=]*[=]{1} *\\-?[0-9]+", std::regex_constants::ECMAScript | std::regex_constants::optimize | std::regex_constants::icase),
      m_isDoubleRegex("[a-z][^=]*[=]{1} *\\-?[0-9]+\\.{1}[0-9]+",
                      std::regex_constants::ECMAScript | std::regex_constants::optimize | std::regex_constants::icase),
      m_isBoolRegex("[a-z][^=]*[=]{1} *(true|false)", std::regex_constants::ECMAScript | std::regex_constants::optimize | std::regex_constants::icase)
{
    // noop
}
//=============================================================================
IniDataFile::IniDataFile()
    : m_file(),
      m_isValidRegex("[a-z][^=]*[=]{1} *[^=]+", std::regex_constants::ECMAScript | std::regex_constants::optimize | std::regex_constants::icase),
      m_isIntegerRegex("[a-z][^=]*[=]{1} *\\-?[0-9]+", std::regex_constants::ECMAScript | std::regex_constants::optimize | std::regex_constants::icase),
      m_isDoubleRegex("[a-z][^=]*[=]{1} *\\-?[0-9]+\\.{1}[0-9]+",
                      std::regex_constants::ECMAScript | std::regex_constants::optimize | std::regex_constants::icase),
      m_isBoolRegex("[a-z][^=]*[=]{1} *(true|false)", std::regex_constants::ECMAScript | std::regex_constants::optimize | std::regex_constants::icase)
{
    // noop
}
//=============================================================================
IniDataFile::~IniDataFile()
{
    // noop
}
//=============================================================================
void IniDataFile::setFileName(const std::string& fileName) { m_file.setFileName(fileName); }
//=============================================================================
bool IniDataFile::load()
{
    m_file.setOpenMode(FOM_Read);

    if (!m_file.open())
    {
        return false;
    }

    m_entries.clear();
    bool isValid = true;

    while (true)
    {
        std::string line;

        if (!m_file.readLine(line))  // EOF or ERROR
        {
            break;
        }

        core::CerberusUtils::removeBlank(line);

        if (line.empty())
        {
            continue;
        }

        if (_isValid(line))
        {
            Entry entry;
            entry.key         = _getKey(line);
            entry.stringValue = _getValue(line);
            entry.type        = IniDataType::IDT_String;

            if (_isInteger(line))
            {
                entry.integerValue = std::stoll(entry.stringValue);
                entry.type |= IniDataType::IDT_Integer;
            }

            if (_isDouble(line))
            {
                entry.doubleValue = std::stod(entry.stringValue);
                entry.type |= IniDataType::IDT_Double;
            }

            if (_isBool(line))
            {
                std::string boolean = core::CerberusUtils::toLower(entry.stringValue);
                entry.type |= IniDataType::IDT_Bool;

                if (boolean.compare("true") == 0)
                {
                    entry.boolValue = true;
                }
                else
                {
                    entry.boolValue = false;
                }
            }

            Entry* found = _search(entry.key);

            if (found == nullptr)
            {
                m_entries.push_back(entry);
            }
            else
            {
                *found = entry;
            }
        }
        else
        {
            isValid = false;
        }
    }

    m_file.close();
    return isValid;
}
//=============================================================================
bool IniDataFile::exists(const std::string& key) { return (_search(key) != nullptr); }
//=============================================================================
uint8_t IniDataFile::type(const std::string& key)
{
    Entry* found = _search(key);

    if (found == nullptr)
    {
        return IniDataType::IDT_NotAType;
    }

    return found->type;
}
//=============================================================================
bool IniDataFile::write_string(const std::string& key, const std::string& value)
{
    std::string k    = core::CerberusUtils::removeBlank_copy(key);
    std::string v    = core::CerberusUtils::removeBlank_copy(value);
    std::string line = core::CerberusUtils::strPrint("%s = %s", k.c_str(), v.c_str());

    if (!_isValid(line))
    {
        // throw cerberusIllegalArgExc("Provided key=value string pair is not valid");
        return false;
    }

    Entry* found = _search(k);

    if (found == nullptr)
    {
        Entry newEntry;
        newEntry.key         = k;
        newEntry.stringValue = v;
        newEntry.type        = IniDataType::IDT_String;
        m_entries.push_back(newEntry);
    }
    else
    {
        if (found->type & IniDataType::IDT_String)
        {
            found->stringValue = v;
        }
        else
        {
            // throw cerberusIllegalArgExc("Value of provided key does not match data type");
            return false;
        }
    }

    _syncFile();
    return true;
}
//=============================================================================
bool IniDataFile::write_integer(const std::string& key, int64_t value)
{
    std::string k        = core::CerberusUtils::removeBlank_copy(key);
    std::string strValue = core::CerberusUtils::strPrint("%i", value);
    std::string line     = core::CerberusUtils::strPrint("%s = %s", k.c_str(), strValue.c_str());

    if (!_isValid(line))
    {
        // throw cerberusIllegalArgExc("Provided key=value string pair is not valid");
        return false;
    }

    if (!_isInteger(line))
    {
        // throw cerberusIllegalArgExc("Provided key=value string pair is not a valid integer entry");
        return false;
    }

    Entry* found = _search(k);

    if (found == nullptr)
    {
        Entry newEntry;
        newEntry.key          = k;
        newEntry.stringValue  = strValue;
        newEntry.integerValue = value;
        newEntry.type         = IniDataType::IDT_String | IniDataType::IDT_Integer;
        m_entries.push_back(newEntry);
    }
    else
    {
        if (found->type & IniDataType::IDT_Integer)
        {
            found->stringValue  = strValue;
            found->integerValue = value;
        }
        else
        {
            // throw cerberusIllegalArgExc("Value of provided key does not match data type");
            return false;
        }
    }

    _syncFile();

    return true;
}
//=============================================================================
bool IniDataFile::write_double(const std::string& key, double value)
{
    std::string k        = core::CerberusUtils::removeBlank_copy(key);
    std::string strValue = core::CerberusUtils::strPrint("%f", value);
    std::string line     = core::CerberusUtils::strPrint("%s = %s", k.c_str(), strValue.c_str());

    if (!_isValid(line))
    {
        // throw cerberusIllegalArgExc("Provided key=value string pair is not valid");
        return false;
    }

    if (!_isDouble(line))
    {
        // throw cerberusIllegalArgExc("Provided key=value string pair is not a valid double entry");
        return false;
    }

    Entry* found = _search(k);

    if (found == nullptr)
    {
        Entry newEntry;
        newEntry.key         = k;
        newEntry.stringValue = strValue;
        newEntry.doubleValue = value;
        newEntry.type        = IniDataType::IDT_String | IniDataType::IDT_Double;
        m_entries.push_back(newEntry);
    }
    else
    {
        if (found->type & IniDataType::IDT_Double)
        {
            found->stringValue = strValue;
            found->doubleValue = value;
        }
        else
        {
            // throw cerberusIllegalArgExc("Value of provided key does not match data type");
            return false;
        }
    }

    _syncFile();

    return true;
}
//=============================================================================
bool IniDataFile::write_bool(const std::string& key, bool value)
{
    std::string k        = core::CerberusUtils::removeBlank_copy(key);
    std::string strValue = core::CerberusUtils::strPrint("%s", value ? "true" : "false");
    std::string line     = core::CerberusUtils::strPrint("%s = %s", k.c_str(), strValue.c_str());

    if (!_isValid(line))
    {
        // throw cerberusIllegalArgExc("Provided key=value string pair is not valid");
        return false;
    }

    if (!_isBool(line))
    {
        // throw cerberusIllegalArgExc("Provided key=value string pair is not a valid bool entry");
        return false;
    }

    Entry* found = _search(k);

    if (found == nullptr)
    {
        Entry newEntry;
        newEntry.key         = k;
        newEntry.stringValue = strValue;
        newEntry.boolValue   = value;
        newEntry.type        = IniDataType::IDT_String | IniDataType::IDT_Bool;
        m_entries.push_back(newEntry);
    }
    else
    {
        if (found->type & IniDataType::IDT_Bool)
        {
            found->stringValue = strValue;
            found->boolValue   = value;
        }
        else
        {
            // throw cerberusIllegalArgExc("Value of provided key does not match data type");
            return false;
        }
    }

    _syncFile();

    return true;
}
//=============================================================================
std::string IniDataFile::read_string(const std::string& key)
{
    Entry* found = _search(key);

    if (found == nullptr)
    {
        // throw cerberusIllegalArgExc("Provided key does not exist");
        return "";
    }

    if (!(found->type & IniDataType::IDT_String))
    {
        // throw cerberusIllegalArgExc("Value of provided key does not match data type");
        return "";
    }

    return found->stringValue;
}
//=============================================================================
int64_t IniDataFile::read_integer(const std::string& key)
{
    Entry* found = _search(key);

    if (found == nullptr)
    {
        // throw cerberusIllegalArgExc("Provided key does not exist");
        return 0;
    }

    if (!(found->type & IniDataType::IDT_Integer))
    {
        // throw cerberusIllegalArgExc("Value of provided key does not match data type");
        return 0;
    }

    return found->integerValue;
}
//=============================================================================
double IniDataFile::read_double(const std::string& key)
{
    Entry* found = _search(key);

    if (found == nullptr)
    {
        // throw cerberusIllegalArgExc("Provided key does not exist");
        return 0.0f;
    }

    if (!(found->type & IniDataType::IDT_Double))
    {
        // throw cerberusIllegalArgExc("Value of provided key does not match data type");
        return 0.0f;
    }

    return found->doubleValue;
}
//=============================================================================
bool IniDataFile::read_bool(const std::string& key)
{
    Entry* found = _search(key);

    if (found == nullptr)
    {
        // throw cerberusIllegalArgExc("Provided key does not exist");
        return false;
    }

    if (!(found->type & IniDataType::IDT_Bool))
    {
        // throw cerberusIllegalArgExc("Value of provided key does not match data type");
        return false;
    }

    return found->boolValue;
}
//=============================================================================
