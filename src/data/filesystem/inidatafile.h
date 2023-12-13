#ifndef CERBERUS_DATA_FILESYSTEM_INIDATAFILE_H
#define CERBERUS_DATA_FILESYSTEM_INIDATAFILE_H

#define MAIN_SECTION "MAIN"

#include <regex>

#include "file.h"

namespace cerberus
{
    namespace data
    {
        namespace filesystem
        {
            class CERBERUS_EXPORT IniDataFile
            {
               private:
                File m_file;

                std::regex m_isValidRegex;
                std::regex m_isIntegerRegex;
                std::regex m_isDoubleRegex;
                std::regex m_isBoolRegex;

                bool isValid(const std::string& line);

                bool isInteger(const std::string& line);

                bool isDouble(const std::string& line);

                bool isBool(const std::string& line);

                std::string getKey(const std::string& line);

                std::string getValue(const std::string& line);

                struct Entry
                {
                    std::string section;
                    std::string key;
                    uint8_t type;
                    std::string stringValue;
                    union
                    {
                        int64_t integerValue;
                        double doubleValue;
                        bool boolValue;
                    };
                };

                Entry* search(const std::string& key, const std::string& section = MAIN_SECTION);

                static bool exists(const std::vector<std::string>& v, const std::string& str);

                void sort();

                bool syncFile();  // rewrites the file reading from memory

                std::vector<Entry> m_entries;

               public:
                // Construct an IniDataFile object with a file name
                IniDataFile(const std::string& fileName = std::string(""));

                // Close the file if open and destroy the instance
                ~IniDataFile();

                // Sets a file name for the .ini file.
                void setFileName(const std::string& fileName);

                // Returns true if the file is a valid .ini file or if it is empty.
                // When an invalid line is found, the method will still attempt to load the following valid ones, and return false.
                // Calling this method will result in a synchronization between file and memory.
                // Entries that are in the memory but not in the file will be lost.
                // This method open the .ini file in read-only mode
                bool load();

                // Checks if a key exists in the memory
                bool exists(const std::string& key);

                // Returns an OR of possible data types of a key. Values used are defined in DataType enum.
                // Returns DataType::DT_NotAType if provided key was not found
                uint8_t type(const std::string& key);

                // Write methods:
                // These methods perform a write operation on the file (and memory), modifying or adding a key=value entry.
                // If provided key exists, its value will be modified.
                // In this case it is important to ensure that the type is correct using type().
                // If provided key does not exist, it will be created and added to the memory and file.
                // If provided key=value string pair is not valid, the method will return false
                OperationResult write_string(const std::string& key, const std::string& value, const std::string& section = MAIN_SECTION);

                OperationResult write_integer(const std::string& key, int64_t value, const std::string& section = MAIN_SECTION);

                OperationResult write_double(const std::string& key, double value, const std::string& section = MAIN_SECTION);

                OperationResult write_bool(const std::string& key, bool value, const std::string& section = MAIN_SECTION);

                // Read methods:
                // These methods perform a read operation on the memory copied using load().
                // Please ensure the data exists and is type-correct using exists() and type()
                OperationResult read_string(const std::string& key, const std::string& section = MAIN_SECTION);

                OperationResult read_integer(const std::string& key, const std::string& section = MAIN_SECTION);

                OperationResult read_double(const std::string& key, const std::string& section = MAIN_SECTION);

                OperationResult read_bool(const std::string& key, const std::string& section = MAIN_SECTION);
            };
        }  // namespace filesystem
    }      // namespace data
}  // namespace cerberus

#endif  // CERBERUS_DATA_FILESYSTEM_INIDATAFILE_H
