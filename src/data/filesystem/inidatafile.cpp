#include "inidatafile.h"

#include <boost/regex.hpp>

#include "../../core/cerberusutils.h"
#include "../../types.h"
#include "src/cerberus.h"

using namespace crb;

//=============================================================================
bool IniDataFile::isValid(const std::string& line) { return boost::regex_match(line, m_isValidRegex); }
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
crb::DataType IniDataFile::valueType(const std::string& value) { return Opaque(value).type(); }
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
crb::OpRes IniDataFile::syncFile()
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

        if (!el.comment.empty())
            line += CerberusUtils::strPrint(" #%s", el.comment.c_str());  // fix the space

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
        toPrint.append(CerberusUtils::strPrint("%i %i %s %s %s\n", el.sectionSpecifier, el.sectionId,
                                               el.key.c_str(), el.value.c_str(), el.comment.c_str()));
    }

    toPrint.append("\n");

    lldebug("%s", toPrint.c_str());
}
//=============================================================================
IniDataFile::IniDataFile(const std::string& fileName)
    : m_file(fileName),
      m_isValidRegex("[a-z][^=]*[=]{1} *[^=]+",
                     boost::regex::ECMAScript | boost::regex::optimize | boost::regex::icase)
{
    // noop
}
//=============================================================================
IniDataFile::~IniDataFile()
{
    // noop
}
//=============================================================================
void IniDataFile::setFileName(const std::string& fileName) { m_file.path(fileName); }
//=============================================================================
OpRes IniDataFile::load()
{
    m_file.setOpenMode(FOM_Read);

    condret(m_file.open());

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

            OpRes out = OR_OK;
            if (!ok) out.addOptional(OR_Failure);
            return out;  // EOF is ok
        }

        std::string line = res.value;

        CerberusUtils::removeBlank(line);

        if (line.empty()) continue;

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
bool IniDataFile::exists(const std::string& key, const std::string& section)
{
    return (search(key, section) != nullptr);
}
//=============================================================================
bool IniDataFile::isType(const std::string& key, DataType type) { return (this->type(key) == type); }
//=============================================================================
crb::DataType IniDataFile::type(const std::string& key, const std::string& section)
{
    auto found = search(key, section);

    if (found == nullptr) return DT_Invalid;

    return valueType(found->value);
}
//=============================================================================
crb::OpRes IniDataFile::rewrite() { return syncFile(); }
//=============================================================================
crb::OpRes IniDataFile::write(const std::string& key, const Opaque& value, const std::string& section)
{
    std::string k = CerberusUtils::removeBlank_copy(key);
    std::string v = CerberusUtils::removeBlank_copy(value.get());

    if (k.empty() || v.empty()) return OR_WrongArgument;

    DataType type = value.type();
    if (type == DT_Invalid) type = valueType(v);

    Line* found = search(k, section);

    if (found == nullptr)
    {
        auto sectionId = addNewSection(section);  // add OR get an existing one
        Line newLine;
        newLine.sectionId = sectionId;
        newLine.key       = k;
        newLine.value     = v;
        insertLine(newLine);
    }
    else
    {
        // keep type consistency when both sides have a concrete type
        auto existingType = valueType(found->value);
        if (type != DT_Invalid && existingType != DT_Invalid && existingType != type) return OR_WrongType;
        found->value = v;
    }

    return syncFile();
}
//=============================================================================
crb::OpRes IniDataFile::enforce(const std::string& key, const Opaque& value, const std::string& section)
{
    if (key.empty() || value.get().empty()) return OR_WrongArgument;

    auto res = read(key, section);

    if (res.fail()) return write(key, value, section);

    // key exists: ensure type consistency
    DataType expected = value.type() == DT_Double    ? DT_Double
                        : value.type() == DT_Integer ? DT_Integer
                        : value.type() == DT_Bool    ? DT_Bool
                                                     : DT_Invalid;

    if (expected == DT_Invalid) expected = valueType(value.get());

    if (!isType(key, expected)) return write(key, value, section);

    return OR_OK;
}
//=============================================================================
OpResData<Opaque> IniDataFile::read(const std::string& key, const std::string& section)
{
    auto found = search(key, section);

    if (found == nullptr)
    {
        return OR_NotFound;
    }

    return OpResData<Opaque>(Opaque(found->value));
}
//=============================================================================
