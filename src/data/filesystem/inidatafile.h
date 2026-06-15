#ifndef CERBERUS_DATA_FILESYSTEM_INIDATAFILE_H
#define CERBERUS_DATA_FILESYSTEM_INIDATAFILE_H

#include <list>
#define MAIN_SECTION "MAIN"

#include "../../core/cerberusregex.h"
#include "file.h"

namespace crb
{
    class IniDataFile
    {
       private:
        File m_file;

        CerberusRegex m_isValidRegex;

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
        Line* search(const std::string& key, int16_t sectionId);
        Line* search(const std::string& key, const std::string& section);

        std::string getSectionName(int16_t sectionId);
        int16_t getSectionId(const std::string& name);
        int16_t addNewSection(const std::string& name);

        DataType valueType(const std::string& value);

        void insertLine(const Line& line);

        OpRes syncFile();  // rewrites the file using cached values

        void printDebug();

       public:
        // Construct an IniDataFile object with a file name
        CERBERUS_EXPORT IniDataFile(const std::string& fileName = std::string(""));

        // Close the file if open
        CERBERUS_EXPORT ~IniDataFile();

        // Sets a file name for the .ini file.
        CERBERUS_EXPORT void setFileName(const std::string& fileName);

        // Attempt to load an .ini file.
        // If load() manages to parse all the file with no errors, the result is OR_OK.
        // If load() finds errors while parsing the file, the result is still OR_OK but it has the
        // optional OR_Failure, to indicate that the operation retrieved all possible information
        // from the file but some data was not correctly loaded and has been discarded.
        // If a not ignoreable error has been encountered, OR_Failure is returned.
        // If the given path is not valid, OR_InvalidFile is returned.
        CERBERUS_EXPORT OpRes load();

        // Checks if a key exists in the memory
        CERBERUS_EXPORT bool exists(const std::string& key,
                                    const std::string& section = MAIN_SECTION);

        // Returns the value type of the requested key.
        // If key was not found, IDT_Invalid is returned
        CERBERUS_EXPORT DataType type(const std::string& key,
                                      const std::string& section = MAIN_SECTION);

        // Check if the key object is convertible to the given type
        CERBERUS_EXPORT bool isType(const std::string& key, DataType type);

        // Force the re-write of the entire file
        CERBERUS_EXPORT OpRes rewrite();

        // Write one key with an Opaque value; creates section if missing.
        CERBERUS_EXPORT OpRes write(const std::string& key, const Opaque& value,
                                    const std::string& section = MAIN_SECTION);

        // Enforce a key/value with Opaque. If key exists with different type it is overwritten.
        CERBERUS_EXPORT OpRes enforce(const std::string& key, const Opaque& value,
                                      const std::string& section = MAIN_SECTION);

        // Read a key into an Opaque value.
        CERBERUS_EXPORT OpResData<Opaque> read(const std::string& key,
                                               const std::string& section = MAIN_SECTION);
    };

}  // namespace crb

#endif  // CERBERUS_DATA_FILESYSTEM_INIDATAFILE_H
