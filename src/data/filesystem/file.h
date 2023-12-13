#ifndef CERBERUS_DATA_FILESYSTEM_FILE_H
#define CERBERUS_DATA_FILESYSTEM_FILE_H

#include <stdio.h>

#include "../../Cerberus_global.h"
#include "../../data/bytebuffer.h"
#include "../../types.h"

namespace cerberus
{
    namespace network
    {
        class Socket;
    }

    namespace data
    {
        class SharedByteBuffer;

        namespace filesystem
        {
            class CERBERUS_EXPORT File
            {
                friend class ::cerberus::network::Socket;

               private:
                std::string m_filePath;

                bool m_binaryMode;

                FileOpenMode m_openMode;

                FILE* m_file;

                int m_fd;  // automatically updated when open() is called

                std::string getOpenModeString();

               public:
                // Check wether a file exists on filesystem
                static OperationResult existsAsFile(const std::string& path);

                // Check wether a directory exists on filesystem
                static OperationResult existsAsDirectory(const std::string& path);

                // Create a directory
                static OperationResult createDirectory(const std::string& path);

                // Delete a directory (must be empty)
                static OperationResult deleteDirectory(const std::string& path);

                // Check if a given directory is empty
                static OperationResult isEmptyDirectory(const std::string& path);

                // Get the size of the file in the size field of the return value
                static OperationResult sizeOf(const std::string& path);

                // Create a File instance. The openMode parameter can be one of the FileOpenMode values
                File(FileOpenMode openMode = FOM_Read, bool binaryMode = false);

                File(const std::string& filePath, FileOpenMode openMode = FOM_Read, bool binaryMode = false);

                ~File();

                bool setFileName(const std::string& filePath);

                bool setOpenMode(FileOpenMode openMode, bool binaryMode = false);

                bool canWrite() const;

                std::string fileName() const;

                bool isOpen() const;

                bool open();

                bool close();

                bool deleteFromDisk();

                bool move(const std::string& newName);

                uint64_t size() const;

                bool write(const cerberus::data::ByteBuffer& bytes);

                bool writeLine(const std::string& line = "");

                // Read the file starting from start pos till the end of file
                bool read(ByteBuffer& bytes, uint64_t start = 0) const;

                // Read span bytes from file file starting from start pos
                bool read(ByteBuffer& bytes, uint64_t start, uint64_t span) const;

                bool readChunk(ByteBuffer& bytes, uint64_t chunksize) const;

                // Read a single line till \n
                // Return false when EOF is reached and no more lines are available
                bool readLine(std::string& line) const;

                bool seek(uint64_t pos) const;

                bool seekOffset(int64_t pos) const;

                void resetCursor() const;

                uint64_t getCursor() const;

                bool isEqual(File& other) const;
            };
        }  // namespace filesystem
    }      // namespace data
}  // namespace cerberus

#endif  // CERBERUS_DATA_FILESYSTEM_FILE_H
