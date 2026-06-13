#ifndef CERBERUS_DATA_FILESYSTEM_CSVDATAFILE_H
#define CERBERUS_DATA_FILESYSTEM_CSVDATAFILE_H

#include "../../types.h"
#include "file.h"

#define INVALID_COLUMN (int)-1

namespace crb
{
    class CSVDataFile
    {
        File m_file;                          // underlying file handle
        char m_delim;                         // cache of the used delimiter
        MultiString m_columns;                // cache of the column names
        std::vector<DataType> m_columnTypes;  // cache of the column types
        std::vector<LSIZE> m_records;         // cache of the rows position in file

       public:
        typedef std::vector<Opaque> MultiVal;

        struct CSVRecord
        {
            MultiVal record;

            CERBERUS_EXPORT void addValue(const Opaque& value);
        };

        typedef std::vector<CSVRecord> CSVRecordBlock;

        // Construct a CSVDataFile object. If delim is 0, the delimitator is inferred
        CERBERUS_EXPORT CSVDataFile(const std::string& fileName = std::string(""), char delim = 0);

        ~CSVDataFile() = default;

        // Set filename and delimitator char. if delim is 0, the delimitator is inferred.
        // The file is also kept open from this point
        CERBERUS_EXPORT void setFileName(const std::string& fileName, char delim = 0);

        // Close the file and wipe all the cache
        CERBERUS_EXPORT void close();

        // Parse the file and build the cache structure. Return OR_InvalidFile if the file is malformed (does
        // not have an header), OR_InvalidPath if it does not exist. If load() finds errors while parsing the
        // file, the result is OR_OK but it has the optional OR_Failure, to indicate that the operation
        // retrieved all possible information from the file but some records was not correctly loaded and has
        // been discarded.
        CERBERUS_EXPORT OpRes load();

        // Return the loaded number of records. The header is excluded and not considered a record
        CERBERUS_EXPORT LSIZE size() const;

        // Check if the given column exists and return the index. If the column does not exist, INVALID_COLUMN
        // is returned. The column name must be in the very first row (header).
        // Text matching is case sensitive
        CERBERUS_EXPORT int columnPos(const std::string& col) const;

        // Return all the loaded columns
        CERBERUS_EXPORT MultiString columns() const;

        // Get the type of the value
        CERBERUS_EXPORT DataType type(SIZE columnIndex) const;

        // Get one single value. RecordIndex starts after the header
        CERBERUS_EXPORT Opaque read(LSIZE recordIndex, SIZE columnIndex) const;

        // Set one single value (already existing).
        // RecordIndex starts after the header. This API writes on file
        CERBERUS_EXPORT void write(LSIZE recordIndex, SIZE columnIndex, const Opaque& value);

        // Append one record at the end of the file. Data is validated and an
        // exception is thrown if validation fails
        CERBERUS_EXPORT void addRecord(const CSVRecord& record);

        // Append a block of records. Data is validated and an
        // exception is thrown if validation fails
        CERBERUS_EXPORT void addRecordBlock(const CSVRecordBlock& block);

        // Insert one record at the given index. Exception is thrown if the index is too large
        CERBERUS_EXPORT void insertRecord(const CSVRecord& record, LSIZE recordIndex);

        // Insert a block of records at the given index. Exception is thrown if the index is too large
        CERBERUS_EXPORT void insertRecordBlock(const CSVRecordBlock& record, LSIZE recordIndex);

        // Get one record from file
        CERBERUS_EXPORT CSVRecord getRecord(LSIZE recordIndex);

        // Get a block of records. If span would go EOF, just the available records will be returned.
        // If span is 0, all the records until EOF will be returned
        CERBERUS_EXPORT CSVRecordBlock getRecordBlock(LSIZE recordIndex, LSIZE span = 0);

        // Get all the records of an entire column. Column name is excluded
        CERBERUS_EXPORT MultiVal getColumn(SIZE index);
        CERBERUS_EXPORT MultiVal getColumn(const std::string& headerName);
    };

}  // namespace crb

#endif  // CERBERUS_DATA_FILESYSTEM_CSVDATAFILE_H
