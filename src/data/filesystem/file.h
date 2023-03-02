#ifndef CERBERUS_DATA_FILESYSTEM_FILE_H
#define CERBERUS_DATA_FILESYSTEM_FILE_H

#include <string>
#include <fstream>
#include "../../Cerberus_global.h"
#include "../../define.h"

namespace cerberus
{
    namespace data
    {
        class ByteBuffer;

        namespace filesystem
        {
            class CERBERUS_EXPORT File
            {
                private:
                    std::string m_filePath;

                    std::fstream m_stream;

                    std::ios_base::openmode m_openMode;

                public:
                    //Checks wether a file exists on filesystem
                    //Throws an exception if the operation was not completed successfully
                    static bool existsAsFile(const std::string& path);

                    //Checks wether a directory exists on filesystem
                    //Throws an exception if the operation was not completed successfully
                    static bool existsAsDirectory(const std::string& path);

                    //Creates a directory
                    //Throws an exception if the operation was not completed successfully
                    static void createDirectory(const std::string& path);

                    //Deletes a directory (must be empty)
                    //Throws an exception if the operation was not completed successfully
                    static void deleteDirectory(const std::string& path);

                    //Checks if a given directory is empty
                    //Throws an exception if the operation was not completed successfully
                    static bool isEmptyDirectory(const std::string& path);

                    File(uint8_t openMode = 0); //Default: read-only

                    File(const std::string& filePath, uint8_t openMode = 0);

                    ~File();

                    void setFileName(const std::string& filePath);

                    std::string fileName() const;

                    void setOpenMode(uint8_t openMode = 0);

                    bool isOpen() const;

                    bool open();

                    bool close();

                    bool deleteFromDisk();

                    bool rename(const std::string& newName);

                    uint64_t size();

                    bool write(const ByteBuffer& bytes);

                    bool writeLine(const std::string& line);

                    void read(ByteBuffer& bytes, std::streampos start = 0);

                    void read(ByteBuffer& bytes, std::streampos start, std::streamsize span);

                    //Returns false when EOF is reached and no more lines are available
                    bool readLine(std::string& line);

                    void resetReadCursor();

                    void resetWriteCursor();

                    std::streampos readCursor();

                    std::streampos writeCursor();

                    void setReadCursor(std::streampos pos);

                    void setWriteCursor(std::streampos pos);

                    void moveReadCursor(std::streamoff offset);

                    void moveWriteCursor(std::streamoff offset);
            };
        }
    }
}

#endif // CERBERUS_DATA_FILESYSTEM_FILE_H
