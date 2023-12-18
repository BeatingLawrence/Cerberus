#include "inidatafile.h"

#include "../../core/cerberusutils.h"
#include "../../types.h"
#include "src/cerberus.h"

using namespace cerberus::data::filesystem;
using namespace cerberus::core;

//=============================================================================
bool IniDataFile::isValid(const std::string& line) { return std::regex_match(line, m_isValidRegex); }
//=============================================================================
bool IniDataFile::isInteger(const std::string& val) { return std::regex_match(val, m_isIntegerRegex); }
//=============================================================================
bool IniDataFile::isDouble(const std::string& val) { return std::regex_match(val, m_isDoubleRegex); }
//=============================================================================
bool IniDataFile::isBool(const std::string& val) { return std::regex_match(val, m_isBoolRegex); }
//=============================================================================
IniDataFile::Line* IniDataFile::search(const std::string& key, int16_t sectionId)
{
    for (auto& el : m_lines)
    {
        if (CerberusUtils::areEqual(el.key, key) && el.sectionId == sectionId)
        {
            return &el;
        }
    }

    return nullptr;
}
//=============================================================================
IniDataFile::Line* IniDataFile::search(const std::string& key, const std::string& section)
{
    auto sectionId = getSectionId(section);
    if (sectionId == -1)
    {
        return nullptr;
    }

    return search(key, sectionId);
}
//=============================================================================
std::string IniDataFile::getSectionName(int16_t sectionId)
{
    for (auto&& el : m_sections)
    {
        if (el.id == sectionId)
        {
            return el.name;
        }
    }

    return "UNKNOWN_SECTION";  // it should never happen
}
//=============================================================================
int16_t IniDataFile::getSectionId(const std::string& name)
{
    auto str = name;
    CerberusUtils::removeBlank(str);

    if (str.empty() || CerberusUtils::areEqual(name, MAIN_SECTION, WM_CaseInsensitive))
    {
        return 0;  // main section
    }

    for (auto&& el : m_sections)
    {
        if (CerberusUtils::areEqual(el.name, str))
        {
            return el.id;
        }
    }

    return -1;
}
//=============================================================================
int16_t IniDataFile::addNewSection(const std::string& name)
{
    auto str = name;
    CerberusUtils::removeBlank(str);

    if (str.empty() || CerberusUtils::areEqual(name, MAIN_SECTION, WM_CaseInsensitive))
    {
        return 0;  // main section
    }

    for (auto&& el : m_sections)
    {
        if (CerberusUtils::areEqual(el.name, str))
        {
            return el.id;
        }
    }

    int16_t newId = 1;
    bool notfound = true;

    while (notfound)
    {
        notfound = false;

        for (auto&& el : m_sections)
        {
            if (newId == el.id)
            {
                newId++;
                notfound = true;
                break;
            }
        }
    }

    m_sections.push_back({newId, str});
    return newId;
}
//=============================================================================
cerberus::IniDataType IniDataFile::valueType(const std::string& value)
{
    if (isDouble(value))
        return IDT_Double;
    else if (isInteger(value))
        return IDT_Integer;
    else if (isBool(value))
        return IDT_Bool;

    return IDT_Invalid;
}
//=============================================================================
void IniDataFile::insertLine(const Line& line)
{
    // if the section is the main section, insert at the top of the file

    if (line.sectionId == 0)  // main section
    {
        m_lines.push_front(line);
        return;
    }

    // verify if the section specifier is present

    uint32_t pos = 0;

    for (auto&& el : m_lines)
    {
        if (el.sectionSpecifier && el.sectionId == line.sectionId) break;
        pos++;
    }

    if (pos == m_lines.size())
    {
        // not found, adding
        Line specifier;
        specifier.sectionSpecifier = true;
        specifier.sectionId        = line.sectionId;
        // insert at the end
        m_lines.push_back(specifier);
        m_lines.push_back(line);
        return;
    }

    // found

    uint32_t reached = 0;
    pos++;  // select the NEXT line

    for (auto it = m_lines.begin(); it != m_lines.end(); it++)
    {
        if (reached == pos)
        {
            m_lines.insert(it, line);
            return;
        }

        reached++;
    }
}
//=============================================================================
cerberus::OperationResult IniDataFile::syncFile()
{
    m_file.setOpenMode(FOM_ReadWriteTrunc);

    auto res = m_file.open();

    if (res.fail()) return res;

    if (m_lines.empty())
    {
        m_file.close();  // file content discarded (truncate)
        return OR_OK;
    }

    for (auto&& el : m_lines)
    {
        std::string line;

        if (el.sectionSpecifier)
            line = CerberusUtils::strPrint("[%s]", getSectionName(el.sectionId).c_str());

        else if (!el.key.empty())
            line = CerberusUtils::strPrint("%s = %s", el.key.c_str(), el.value.c_str());

        if (!el.comment.empty()) line += CerberusUtils::strPrint(" #%s", el.comment.c_str());  // fix the space

        m_file.writeLine(line);
    }

    m_file.close();

    return OR_OK;
}
//=============================================================================
void IniDataFile::printDebug()
{
    std::string toPrint;

    toPrint.append("sections:\n");

    for (auto&& el : m_sections)
    {
        toPrint.append(CerberusUtils::strPrint("%i, %s\n", el.id, el.name.c_str()));
    }

    toPrint.append("\nlines:\n");

    for (auto&& el : m_lines)
    {
        toPrint.append(CerberusUtils::strPrint("%i %i %s %s %s\n", el.sectionSpecifier, el.sectionId, el.key.c_str(), el.value.c_str(), el.comment.c_str()));
    }

    toPrint.append("\n");

    lldebug("%s", toPrint.c_str());
}
//=============================================================================
IniDataFile::IniDataFile(const std::string& fileName)
    : m_file(fileName),
      m_isValidRegex("[a-z][^=]*[=]{1} *[^=]+", std::regex_constants::ECMAScript | std::regex_constants::optimize | std::regex_constants::icase),
      m_isIntegerRegex("\\d+", std::regex_constants::ECMAScript | std::regex_constants::optimize | std::regex_constants::icase),
      m_isDoubleRegex("\\d+\\.\\d+", std::regex_constants::ECMAScript | std::regex_constants::optimize | std::regex_constants::icase),
      m_isBoolRegex("(true|false)", std::regex_constants::ECMAScript | std::regex_constants::optimize | std::regex_constants::icase)
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
cerberus::OperationResult IniDataFile::load()
{
    m_file.setOpenMode(FOM_Read);

    auto res = m_file.open();

    if (res.fail()) return res;

    m_lines.clear();
    m_sections.clear();
    bool ok                   = true;
    uint16_t currentSectionId = 0;  // main section is default

    while (true)
    {
        auto res = m_file.readLine();

        if (res.fail())  // EOF or ERROR
        {
            m_file.close();

            if (res.res == OR_Failure)
            {
                m_lines.clear();
                m_sections.clear();
                return OR_Failure;
            }

            return (int64_t)ok;  // EOF is ok
        }

        std::string line = res.str;

        CerberusUtils::removeBlank(line);

        if (line.empty())
        {
            continue;
        }

        Line newLine;
        auto splitted = CerberusUtils::split(line, "#");
        CerberusUtils::removeBlank(splitted.left);
        CerberusUtils::removeBlank(splitted.right);

        newLine.comment   = splitted.right;
        newLine.sectionId = currentSectionId;

        // section check

        if (CerberusUtils::startsWith(splitted.left, '[') && CerberusUtils::endsWith(splitted.left, ']'))
        {
            // line is a section specifier
            splitted.left.erase(0, 1);                      // remove [
            splitted.left.erase(splitted.left.size() - 1);  // remove ]
            newLine.sectionId        = addNewSection(splitted.left);
            newLine.sectionSpecifier = true;
            currentSectionId         = newLine.sectionId;
        }
        else if (isValid(splitted.left))
        {
            auto data = CerberusUtils::split(splitted.left, "=");
            CerberusUtils::removeBlank(data.left);
            CerberusUtils::removeBlank(data.right);
            newLine.key   = data.left;
            newLine.value = data.right;

            if (search(newLine.key, currentSectionId))
            {
                // duplicate
                logError("Duplicate key: %s", line.c_str());
                ok = false;
                continue;
            }
        }
        else if (!splitted.left.empty())
        {
            // non-comment part is not empty but not recognized
            // so at least one line contains invalid data
            logError("Syntax error in .ini file: %s", line.c_str());
            ok = false;
            continue;
        }

        m_lines.push_back(newLine);
    }
}
//=============================================================================
bool IniDataFile::exists(const std::string& key, const std::string& section) { return (search(key, section) != nullptr); }
//=============================================================================
cerberus::IniDataType IniDataFile::type(const std::string& key, const std::string& section)
{
    auto found = search(key, section);

    if (found == nullptr)
    {
        return IDT_Invalid;
    }

    return valueType(found->value);
}
//=============================================================================
cerberus::OperationResult IniDataFile::rewrite() { return syncFile(); }
//=============================================================================
cerberus::OperationResult IniDataFile::write_string(const std::string& key, const std::string& value, const std::string& section)
{
    std::string k = core::CerberusUtils::removeBlank_copy(key);
    std::string v = core::CerberusUtils::removeBlank_copy(value);

    if (k.empty() || v.empty())
    {
        return OR_WrongArgument;
    }

    Line* found = search(k, section);

    if (found == nullptr)
    {
        auto sectionId = addNewSection(section);  // add OR get an existing one
        Line newLine;
        newLine.sectionId = sectionId;
        // newLine.comment
        newLine.key       = k;
        newLine.value     = v;
        insertLine(newLine);
    }
    else
    {
        found->value = v;
    }

    return syncFile();
}
//=============================================================================
cerberus::OperationResult IniDataFile::write_integer(const std::string& key, int64_t value, const std::string& section) { return write_string(key, CerberusUtils::strPrint("%lli", value), section); }
//=============================================================================
cerberus::OperationResult IniDataFile::write_double(const std::string& key, double value, const std::string& section) { return write_string(key, CerberusUtils::strPrint("%f", value), section); }
//=============================================================================
cerberus::OperationResult IniDataFile::write_bool(const std::string& key, bool value, const std::string& section) { return write_string(key, value ? "true" : "false", section); }
//=============================================================================
cerberus::OperationResult IniDataFile::read_string(const std::string& key, const std::string& section)
{
    auto found = search(key, section);

    if (found == nullptr)
    {
        return OR_NotFound;
    }

    return found->value;
}
//=============================================================================
cerberus::OperationResult IniDataFile::read_integer(const std::string& key, const std::string& section)
{
    auto res = read_string(key, section);

    if (res.fail()) return res;

    if (!isInteger(res.str)) return OR_WrongType;

    return (int64_t)CerberusUtils::stringToInt(res.str);
}
//=============================================================================
cerberus::OperationResult IniDataFile::read_double(const std::string& key, const std::string& section)
{
    auto res = read_string(key, section);

    if (res.fail()) return res;

    if (!isDouble(res.str)) return OR_WrongType;

    return CerberusUtils::stringToDouble(res.str);
}
//=============================================================================
cerberus::OperationResult IniDataFile::read_bool(const std::string& key, const std::string& section)
{
    auto res = read_string(key, section);

    if (res.fail()) return res;

    if (!isBool(res.str)) return OR_WrongType;

    if (CerberusUtils::areEqual(CerberusUtils::toLower(res.str), "true")) return (int64_t)1;

    return (int64_t)0;
}
//=============================================================================
