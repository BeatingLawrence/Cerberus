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
                // This method returns OR_OK if the file exists, OR_NotFount if it does not exist or
                // another error if an error occurred
                static OpRes existsAsFile(const std::string& path);

                // Check wether a directory exists on filesystem
                // This method returns OR_OK if the file exists, OR_NotFount if it does not exist or
                // another error if an error occurred
                static OpRes existsAsDirectory(const std::string& path);

                // Create a directory
                static OpRes createDirectory(const std::string& path);

                // Delete a file or directory. If path is a directory, it must be empty
                static OpRes remove(const std::string& path);

                // Move a file referenced by oldPath to newPath
                static OpRes move(const std::string& oldPath, const std::string& newPath);

                // Check if a given directory is empty
                // This method returns OR_OK if the directory is empty,
                // OR_NotEmpty if the directory is not empty, or other values to signal system errors
                static OpRes isEmptyDirectory(const std::string& path);

                // Get the size of the file in the integer field of the return value
                static SizeOpRes sizeOf(const std::string& path);

                // Create a File instance
                File(FileOpenMode openMode = FOM_Read, bool binaryMode = false);
                File(const std::string& filePath, FileOpenMode openMode = FOM_Read, bool binaryMode = false);

                ~File();

                // Set the fileName (path) of the file
                void setFileName(const std::string& filePath);

                // Set the openMode of the file
                void setOpenMode(FileOpenMode openMode, bool binaryMode = false);

                // Know if the instance can write with the currently set openMode
                bool canWrite() const;

                // Get the filename associated to this instance
                std::string fileName() const;

                // Check if the file is currently open
                bool isOpen() const;

                // Open the file with the set file path and open mode.
                // If the path is empty, OR_InvalidPath is returned.
                // If the open fails, OR_Failure is returned, and info about the error
                // are written inside str.
                OpRes open();

                // Close the file if open
                void close();

                // Close the file if open, and remove it from filesystem
                OpRes deleteFromDisk();

                // Move the current file to another path name
                OpRes move(const std::string& newName);

                // Get the file size. Cursor position will not be altered
                SizeOpRes size() const;

                // Write buffer to file
                OpRes write(const cerberus::data::ByteBuffer& bytes);

                // Write a single line of text on file
                OpRes writeLine(const std::string& line = "");

                // Read the file starting from start pos till the end of file
                OpRes read(ByteBuffer& bytes, LSIZE start = 0) const;

                // Read span bytes from file starting from start pos
                OpRes read(ByteBuffer& bytes, LSIZE start, LSIZE span) const;

                // Read a chunk of data from the current cursor position
                OpRes readChunk(ByteBuffer& bytes, SIZE chunksize) const;

                // Read a single line till \n or EOF
                // If the EOF is reached and the bytes read are zero, OR_EOF is returned
                // If an error occurs during read, OR_Failure is returned
                StringOpRes readLine() const;

                // Move the cursor to the absolute position pos
                OpRes seek(LSIZE pos) const;

                // Move the cursor back or forth according to the sign of the parameter pos
                OpRes seekOffset(int64_t pos) const;

                // Reset the cursor moving it to the beginning of the file
                void resetCursor() const;

                // Get the cursor position.
                SizeOpRes getCursor() const;

                // Check if this file and other file are equal (same size, same content)
                BoolOpRes isEqual(File& other) const;
            };
        }  // namespace filesystem
    }      // namespace data
}  // namespace cerberus

#endif  // CERBERUS_DATA_FILESYSTEM_FILE_H
