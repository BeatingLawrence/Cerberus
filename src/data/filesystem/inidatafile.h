#ifndef CERBERUS_DATA_FILESYSTEM_INIDATAFILE_H
#define CERBERUS_DATA_FILESYSTEM_INIDATAFILE_H

#include <list>
#define MAIN_SECTION "MAIN"

#include <regex>

#include "file.h"

namespace crb
{
    class CERBERUS_EXPORT IniDataFile
    {
       private:
        File m_file;

        std::regex m_isValidRegex;
        std::regex m_isIntegerRegex;
        std::regex m_isDoubleRegex;
        std::regex m_isBoolRegex;

        struct Line
        {
            bool sectionSpecifier;
            int16_t sectionId;
            std::string comment;
            std::string key;
            std::string value;

            Line()
                : sectionSpecifier(false),
                  sectionId(0),
                  comment(),
                  key(),
                  value()
            {
            }
        };

        struct Section
        {
            int16_t id;
            std::string name;
        };

        std::list<Line> m_lines;

        std::vector<Section> m_sections;

        bool isValid(const std::string& line);
        bool isInteger(const std::string& val);
        bool isDouble(const std::string& val);
        bool isBool(const std::string& val);

        Line* search(const std::string& key, int16_t sectionId);
        Line* search(const std::string& key, const std::string& section);

        std::string getSectionName(int16_t sectionId);
        int16_t getSectionId(const std::string& name);
        int16_t addNewSection(const std::string& name);

        IniDataType valueType(const std::string& value);

        void insertLine(const Line& line);

        OpRes syncFile();  // rewrites the file using cached values

        void printDebug();

       public:
        // Construct an IniDataFile object with a file name
        IniDataFile(const std::string& fileName = std::string(""));

        // Close the file if open
        ~IniDataFile();

        // Sets a file name for the .ini file.
        void setFileName(const std::string& fileName);

        // Attempt to load an .ini file.
        // If load() manages to parse all the file with no errors, the result is OR_OK.
        // If load() finds errors while parsing the file, the result is still OR_OK but it has the
        // optional OR_Failure, to indicate that the operation retrieved all possible information
        // from the file but some data was not correctly written and has been discarded.
        // If a not ignoreable error has been encountered, OR_Failure is returned.
        // If the given path is not valid, OR_InvalidFile is returned.
        OpRes load();

        // Checks if a key exists in the memory
        bool exists(const std::string& key, const std::string& section = MAIN_SECTION);

        // Returns the value type of the requested key.
        // If key was not found, IDT_Invalid is returned
        IniDataType type(const std::string& key, const std::string& section = MAIN_SECTION);

        // Check if the key object is convertible to the given type
        bool isType(const std::string& key, IniDataType type);

        // Force the re-write of the entire file
        OpRes rewrite();

        // Write methods:
        // These methods synchronously add or modifiy a
        // key=value pair in both the file and memory.
        // If the given section does not exist it will be created as well,
        // and inserted at the end of the file.
        // When the main section is specified, the key=value pair is written
        // at the top of the file instead.
        // If provided key or value is empty, the method will return OR_WrongArgument
        OpRes write_string(const std::string& key, const std::string& value,
                           const std::string& section = MAIN_SECTION);
        OpRes write_integer(const std::string& key, int64_t value, const std::string& section = MAIN_SECTION);
        OpRes write_double(const std::string& key, double value, const std::string& section = MAIN_SECTION);
        OpRes write_bool(const std::string& key, bool value, const std::string& section = MAIN_SECTION);

        // Enforce methods:
        // These methods ensure that a given key exists in the file.
        // If the key already exists, nothing is changed for string values.
        // For typed values, if the key exists but has a different type, it is overwritten.
        // If the key does not exist, it is created as if calling the corresponding write_*().
        OpRes enforce_string(const std::string& key, const std::string& value,
                             const std::string& section = MAIN_SECTION);
        OpRes enforce_integer(const std::string& key, int64_t value,
                              const std::string& section = MAIN_SECTION);
        OpRes enforce_double(const std::string& key, double value,
                             const std::string& section = MAIN_SECTION);
        OpRes enforce_bool(const std::string& key, bool value,
                           const std::string& section = MAIN_SECTION);

        // Read methods:
        // These methods return the requested data reading from the keys loaded with load().
        // If the value does not exist, OR_NotFound is returned.
        // If the value is of a different type, OR_WrongType is returned.
        // read_string() does not check for the type because all values are strings
        StringOpRes read_string(const std::string& key, const std::string& section = MAIN_SECTION);
        IntOpRes read_integer(const std::string& key, const std::string& section = MAIN_SECTION);
        FloatOpRes read_double(const std::string& key, const std::string& section = MAIN_SECTION);
        BoolOpRes read_bool(const std::string& key, const std::string& section = MAIN_SECTION);
    };

}  // namespace crb

#endif  // CERBERUS_DATA_FILESYSTEM_INIDATAFILE_H
