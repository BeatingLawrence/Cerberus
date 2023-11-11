#include "inidatafile.h"

#include "../../core/cerberusutils.h"
#include "../../types.h"

using namespace cerberus::data::filesystem;

//=============================================================================
bool IniDataFile::isValid(const std::string& line) { return std::regex_match(line, m_isValidRegex); }
//=============================================================================
bool IniDataFile::isInteger(const std::string& line) { return std::regex_match(line, m_isIntegerRegex); }
//=============================================================================
bool IniDataFile::isDouble(const std::string& line) { return std::regex_match(line, m_isDoubleRegex); }
//=============================================================================
bool IniDataFile::isBool(const std::string& line) { return std::regex_match(line, m_isBoolRegex); }
//=============================================================================
std::string IniDataFile::getKey(const std::string& line)
{
    for (size_t i = line.find_first_of('=') - 1; i >= 0; i--)
    {
        if (line[i] != ' ')
        {
            return line.substr(0, i + 1);
        }
    }

    return "";
}
//=============================================================================
std::string IniDataFile::getValue(const std::string& line)
{
    for (size_t i = line.find_first_of('=') + 1; i < line.size(); i++)
    {
        if (line[i] != ' ')
        {
            return line.substr(i, std::string::npos);
        }
    }

    return "";
}
//=============================================================================
IniDataFile::Entry* IniDataFile::search(const std::string& key, const std::string& section)
{
    for (auto& el : m_entries)
    {
        if (el.key.compare(key) == 0 && el.section.compare(section) == 0)
        {
            return &el;
        }
    }

    return nullptr;
}
//=============================================================================
bool IniDataFile::exists(const std::vector<std::string>& v, const std::string& str)
{
    for (auto&& el : v)
    {
        if (el.compare(str) == 0)
        {
            return true;
        }
    }

    return false;
}
//=============================================================================
void IniDataFile::sort()
{
    std::vector<std::string> v;
    std::vector<Entry> newEntries;

    for (auto&& el : m_entries)
    {
        // check if the section already esists
        if (!exists(v, el.section))
        {
            if (el.section.compare(MAIN_SECTION) == 0)  // if the section is the main, push at the beginning
                v.insert(v.begin(), el.section);
            else
                v.push_back(el.section);
        }
    }

    for (auto&& sec : v)
    {
        for (auto&& el : m_entries)
        {
            if (sec.compare(el.section) == 0)
            {
                newEntries.push_back(el);
            }
        }
    }

    m_entries = newEntries;
}
//=============================================================================
bool IniDataFile::syncFile()
{
    m_file.setOpenMode(FOM_ReadWriteTrunc);

    if (!m_file.open())
    {
        return false;
    }

    if (m_entries.empty())
    {
        m_file.close();  // content discarded
        return true;
    }

    sort();
    std::string currentSection = m_entries.front().section;  // first section

    if (currentSection.compare(MAIN_SECTION) != 0)
    {
        m_file.writeLine(core::CerberusUtils::strPrint("[%s]", currentSection.c_str()));
    }

    for (auto& el : m_entries)
    {
        if (currentSection.compare(el.section) != 0 && el.section.compare(MAIN_SECTION) != 0)
        {
            m_file.writeLine();  // prints \n
            m_file.writeLine(core::CerberusUtils::strPrint("[%s]", el.section.c_str()));
            currentSection = el.section;
        }

        m_file.writeLine(core::CerberusUtils::strPrint("%s = %s", el.key.c_str(), el.stringValue.c_str()));
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
    bool valid = true;
    std::string section;

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

        if (core::CerberusUtils::startsWith(line, '#'))  // line is a comment
        {
            continue;  // ignore
        }

        // section check

        if (core::CerberusUtils::startsWith(line, '[') && core::CerberusUtils::endsWith(line, ']'))  // line is a section specifier
        {
            section = line;
            section.erase(0, 1);                // remove [
            section.erase(section.size() - 1);  // remove ]
            continue;
        }

        //

        if (!isValid(line))
        {
            valid = false;
            continue;
        }

        Entry entry;
        entry.section     = section.empty() ? MAIN_SECTION : section;
        entry.key         = getKey(line);
        entry.stringValue = getValue(line);
        entry.type        = IniDataType::IDT_String;

        if (isDouble(line))
        {
            entry.doubleValue = std::stod(entry.stringValue);
            entry.type |= IniDataType::IDT_Double;
        }
        else if (isInteger(line))
        {
            entry.integerValue = std::stoll(entry.stringValue);
            entry.type |= IniDataType::IDT_Integer;
        }
        else if (isBool(line))
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

        Entry* found = search(entry.key);

        if (found == nullptr)
        {
            m_entries.push_back(entry);
        }
        else
        {
            *found = entry;
        }
    }

    m_file.close();
    return valid;
}
//=============================================================================
bool IniDataFile::exists(const std::string& key) { return (search(key) != nullptr); }
//=============================================================================
uint8_t IniDataFile::type(const std::string& key)
{
    Entry* found = search(key);

    if (found == nullptr)
    {
        return IniDataType::IDT_NotAType;
    }

    return found->type;
}
//=============================================================================
cerberus::OperationResult IniDataFile::write_string(const std::string& key, const std::string& value, const std::string& section)
{
    std::string k    = core::CerberusUtils::removeBlank_copy(key);
    std::string v    = core::CerberusUtils::removeBlank_copy(value);
    std::string line = core::CerberusUtils::strPrint("%s = %s", k.c_str(), v.c_str());

    if (!isValid(line))
    {
        return OR_WrongArgument;
    }

    Entry* found = search(k, section);

    if (found == nullptr)
    {
        Entry newEntry;
        newEntry.section     = section;
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
            return OR_WrongArgument;
        }
    }

    syncFile();
    return OR_OK;
}
//=============================================================================
cerberus::OperationResult IniDataFile::write_integer(const std::string& key, int64_t value, const std::string& section)
{
    std::string k        = core::CerberusUtils::removeBlank_copy(key);
    std::string strValue = core::CerberusUtils::strPrint("%i", value);
    std::string line     = core::CerberusUtils::strPrint("%s = %s", k.c_str(), strValue.c_str());

    if (!isValid(line))
    {
        return OR_WrongArgument;
    }

    if (!isInteger(line))
    {
        return OR_WrongArgument;
    }

    Entry* found = search(k, section);

    if (found == nullptr)
    {
        Entry newEntry;
        newEntry.section      = section;
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
            return OR_WrongArgument;
        }
    }

    syncFile();

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult IniDataFile::write_double(const std::string& key, double value, const std::string& section)
{
    std::string k        = core::CerberusUtils::removeBlank_copy(key);
    std::string strValue = core::CerberusUtils::strPrint("%f", value);
    std::string line     = core::CerberusUtils::strPrint("%s = %s", k.c_str(), strValue.c_str());

    if (!isValid(line))
    {
        return OR_WrongArgument;
    }

    if (!isDouble(line))
    {
        return OR_WrongArgument;
    }

    Entry* found = search(k, section);

    if (found == nullptr)
    {
        Entry newEntry;
        newEntry.section     = section;
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
            return OR_WrongArgument;
        }
    }

    syncFile();

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult IniDataFile::write_bool(const std::string& key, bool value, const std::string& section)
{
    std::string k        = core::CerberusUtils::removeBlank_copy(key);
    std::string strValue = core::CerberusUtils::strPrint("%s", value ? "true" : "false");
    std::string line     = core::CerberusUtils::strPrint("%s = %s", k.c_str(), strValue.c_str());

    if (!isValid(line))
    {
        return OR_WrongArgument;
    }

    if (!isBool(line))
    {
        return OR_WrongArgument;
    }

    Entry* found = search(k, section);

    if (found == nullptr)
    {
        Entry newEntry;
        newEntry.section     = section;
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
            return OR_WrongArgument;
        }
    }

    syncFile();

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult IniDataFile::read_string(const std::string& key, const std::string& section)
{
    Entry* found = search(key, section);

    if (found == nullptr)
    {
        return OR_WrongArgument;
    }

    if (!(found->type & IniDataType::IDT_String))
    {
        return OR_WrongArgument;
    }

    return found->stringValue;
}
//=============================================================================
cerberus::OperationResult IniDataFile::read_integer(const std::string& key, const std::string& section)
{
    Entry* found = search(key, section);

    if (found == nullptr)
    {
        return OR_WrongArgument;
    }

    if (!(found->type & IniDataType::IDT_Integer))
    {
        return OR_WrongArgument;
    }

    return found->integerValue;
}
//=============================================================================
cerberus::OperationResult IniDataFile::read_double(const std::string& key, const std::string& section)
{
    Entry* found = search(key, section);

    if (found == nullptr)
    {
        return OR_WrongArgument;
    }

    if (!(found->type & IniDataType::IDT_Double))
    {
        return OR_WrongArgument;
    }

    return found->doubleValue;
}
//=============================================================================
cerberus::OperationResult IniDataFile::read_bool(const std::string& key, const std::string& section)
{
    Entry* found = search(key, section);

    if (found == nullptr)
    {
        return OR_WrongArgument;
    }

    if (!(found->type & IniDataType::IDT_Bool))
    {
        return OR_WrongArgument;
    }

    return (int64_t)(found->boolValue);
}
//=============================================================================
