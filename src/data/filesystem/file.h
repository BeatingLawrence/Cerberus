#ifndef CERBERUS_DATA_FILESYSTEM_FILE_H
#define CERBERUS_DATA_FILESYSTEM_FILE_H

#include <stdio.h>

#include "../../Cerberus_global.h"
#include "../../data/bytebuffer.h"
#include "../../types.h"

namespace cerberus
{
    class Socket;

    class Directory;

    class CERBERUS_EXPORT File
    {
        friend class ::cerberus::Socket;
        friend class ::cerberus::Directory;

       private:
        Path m_path;

        bool m_binaryMode;

        FileOpenMode m_openMode;

        FILE* m_file;

        int m_fd;

        std::string getOpenModeString();

       public:
        // Check wether a regular file exists on filesystem
        // This method returns:
        //      OR_OK if the file exists
        //      OR_InvalidPath if it's not a regular file
        //      OR_NotFound if it does not exist
        //      OR_SystemFailure if a system error occurred
        static OpRes existsAsFile(const std::string& path);

        // Check wether a directory exists on filesystem
        // This method returns:
        //      OR_OK if the directory exists
        //      OR_InvalidPath if it's not a directory
        //      OR_NotFound if it does not exist
        //      OR_SystemFailure if a system error occurred
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

        // Stat the file or directory
        static OpResData<FileMetadata> stat(const std::string& path);

        // Create a File instance
        File(FileOpenMode openMode = FOM_Read, bool binaryMode = false);
        File(const Path& path, FileOpenMode openMode = FOM_Read, bool binaryMode = false);

        File(const File& other);
        File(File&& other);

        File& operator=(const File& other);

        ~File();

        // Get file metadata (see FileMetadata structure)
        OpResData<FileMetadata> stat();

        // Set the path of the file
        void path(const Path& path);

        // Set the openMode of the file
        void setOpenMode(FileOpenMode openMode, bool binaryMode = false);

        // Know if the instance can write with the currently set openMode
        bool canWrite() const;

        // Get the filename associated to this instance
        std::string name() const;

        // Get the set path
        Path path() const;

        // Get the complete absolute path of the file
        Path completePath() const;

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
        OpRes remove();

        // Move the current file to another path name
        OpRes move(const Path& newPath);

        // Get the file size
        SizeOpRes size() const;

        // Write buffer to file
        OpRes write(const ByteBuffer& bytes);

        // Write a single line of text on file
        OpRes writeLine(const std::string& line = "");

        // Read the file starting from start pos till the end of file
        OpRes read(ByteBuffer& bytes, LSIZE start = 0) const;

        // Read span bytes from file starting from start pos
        OpRes read(ByteBuffer& bytes, LSIZE start, LSIZE span) const;

        // Read a chunk of data from the current cursor position
        OpRes readChunk(ByteBuffer& bytes, SIZE chunksize) const;

        // Read a chunk of data from the current cursor position until a sequence is found.
        // The read sequence is inserted into the returned buffer
        OpResData<ByteBuffer> readUntil(const ByteBuffer& sequence) const;

        // Advance the cursor until the wanted sequence is found.
        // The position before this call is not reset, thus only the
        // bytes after the cursor position are searched.
        // Please note: this method will return the sequence starting
        // position but will leave the file cursor at the end of the sequence
        // (the byte after the sequence, or EOF)
        SizeOpRes search(const ByteBuffer& sequence) const;

        // Read a single line till \n or EOF
        // If the EOF is reached and the bytes read are zero, OR_EOF is returned
        // If an error occurs during read, OR_Failure is returned
        StringOpRes readLine() const;

        // Move the cursor to the absolute position pos
        OpRes seek(LSIZE pos) const;

        // Move the cursor back or forth according to the sign of the parameter pos
        OpRes seekOffset(int64_t pos) const;

        // Move the cursor to the end of file
        OpRes seekToEOF() const;

        // Reset the cursor moving it to the beginning of the file
        void resetCursor() const;

        // Get the cursor position.
        SizeOpRes getCursor() const;

        // Check if this file and other file are equal (same size, same content)
        BoolOpRes isEqual(File& other) const;
    };
}  // namespace cerberus

#endif  // CERBERUS_DATA_FILESYSTEM_FILE_H
