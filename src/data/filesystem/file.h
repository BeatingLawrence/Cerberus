#ifndef CERBERUS_DATA_FILESYSTEM_FILE_H
#define CERBERUS_DATA_FILESYSTEM_FILE_H

#include <stdio.h>

#include "../../Cerberus_global.h"
#include "../../data/bytebuffer.h"
#include "../../types.h"

namespace crb
{
    class Socket;

    class Directory;

    class File
    {
        friend class ::crb::Socket;
        friend class ::crb::Directory;

       private:
        Path m_path;

        FileOpenMode m_openMode;

        FILE* m_file;

        int m_fd;

        std::string getOpenModeString();

        OpRes _seek(LSIZE pos) const;

        OpRes _read(ByteBuffer& buf, LSIZE start, LSIZE span = 0) const;

        OpRes _read_cursor(ByteBuffer& buf, LSIZE span) const;

        static OpRes _zeroCopy(File& src, File& dst, LSIZE len, bool eof);

        File(int fd, const Path& path,
             FileOpenMode openMode);  // used internally, set openmode to FOM_ReadWrite

       public:
        // Check wether a regular file exists on filesystem
        // This method returns:
        //      OR_OK if the file exists
        //      OR_InvalidPath if it's not a regular file
        //      OR_NotFound if it does not exist
        //      OR_SystemFailure if a system error occurred
        CERBERUS_EXPORT static OpRes existsAsFile(const Path& path);

        // Check wether a directory exists on filesystem
        // This method returns:
        //      OR_OK if the directory exists
        //      OR_InvalidPath if it's not a directory
        //      OR_NotFound if it does not exist
        //      OR_SystemFailure if a system error occurred
        CERBERUS_EXPORT static OpRes existsAsDirectory(const Path& path);

        // Create a directory
        CERBERUS_EXPORT static OpRes createDirectory(const Path& path);

        // Delete a file or directory. If path is a directory, it must be empty
        CERBERUS_EXPORT static OpRes remove(const Path& path);

        // Erase a span of bytes from a file, starting at offset start for length span.
        // The content after the erased block is shifted forward.
        // Uses a temporary file under the same directory and replaces the original on success.
        CERBERUS_EXPORT OpRes erase(LSIZE start, LSIZE span);

        // Move a file referenced by oldPath to newPath
        CERBERUS_EXPORT static OpRes move(const Path& oldPath, const Path& newPath);

        // Check if a given directory is empty
        // This method returns OR_OK if the directory is empty,
        // OR_NotEmpty if the directory is not empty, or other values to signal system errors
        CERBERUS_EXPORT static OpRes isEmptyDirectory(const Path& path);

        // Stat the file or directory
        CERBERUS_EXPORT static OpResData<FileMetadata> stat(const Path& path);

        // Open a new temp file and return it.
        // If path is not empty, it must refer to the directory where
        // the file will be created. If path is empty, P_tmpdir macro will be used
        CERBERUS_EXPORT static File tmpFile(const Path& path = Path(),
                                            FileOpenMode openMode = FOM_ReadWrite);

        // Copy from src file to dst file using their current cursor positions.
        // Linux uses copy_file_range() with a user-space fallback. Windows and
        // MACOSX currently use the portable user-space fallback.
        CERBERUS_EXPORT static OpRes zeroCopy(File& src, File& dst, LSIZE len);
        CERBERUS_EXPORT static OpRes zeroCopy(File& src, File& dst);

        // Create a File instance
        CERBERUS_EXPORT File(FileOpenMode openMode = FOM_Read);
        CERBERUS_EXPORT File(const Path& path, FileOpenMode openMode = FOM_Read);

        CERBERUS_EXPORT File(const File& other);
        CERBERUS_EXPORT File(File&& other);

        CERBERUS_EXPORT File& operator=(const File& other);

        CERBERUS_EXPORT ~File();

        // Get file metadata (see FileMetadata structure)
        CERBERUS_EXPORT OpResData<FileMetadata> stat();

        // Set the path of the file
        CERBERUS_EXPORT void path(const Path& path);

        // Set the open mode of the file. This method will throw
        // an exception if the file is already open
        CERBERUS_EXPORT void setOpenMode(FileOpenMode openMode);

        // Tell if the instance can write with the currently set open mode
        CERBERUS_EXPORT bool canWrite() const;

        // Get the filename associated to this instance
        CERBERUS_EXPORT std::string name() const;

        // Get the set path
        CERBERUS_EXPORT Path path() const;

        // Get the complete absolute path of the file
        CERBERUS_EXPORT Path completePath() const;

        // Get the directory path
        CERBERUS_EXPORT Path directory() const;

        // Check if the file is currently open
        CERBERUS_EXPORT bool isOpen() const;

        // Open the file with the set file path and open mode.
        // If the path is empty, OR_InvalidPath is returned.
        // If the open fails, OR_Failure is returned, and info about the error
        // are written inside str.
        CERBERUS_EXPORT OpRes open();

        // Close the file if open
        CERBERUS_EXPORT void close();

        // Close the current file and reopen it
        CERBERUS_EXPORT OpRes reopen();

        // Close the file if open, and remove it from filesystem
        CERBERUS_EXPORT OpRes remove();

        // Move the current file to another path name
        CERBERUS_EXPORT OpRes move(const Path& newPath);

        // Get the file size
        CERBERUS_EXPORT SizeOpRes size() const;

        // Write buffer to file
        CERBERUS_EXPORT OpRes write(const ByteBuffer& bytes);

        // Write a single line of text on file
        CERBERUS_EXPORT OpRes writeLine(const std::string& line = "");

        // Write the buffer at the end of the file, effectively increasing its size.
        // This method temporary closes and reopen the file with append flag if that
        // was not present while opening. After that, it reopens the file again with the
        // previous open mode.
        CERBERUS_EXPORT OpRes writeExpand(const ByteBuffer& bytes);

        // Insert the given buffer at the cursor position, increasing the file size.
        // This method makes use of tempfile
        CERBERUS_EXPORT OpRes insert(const ByteBuffer& bytes);

        // Read the file starting from start pos till the end of file.
        // If the call reach EOF and was able to read some data, an optional EOF will
        // be added to the return opres. If EOF is reached with no data retrieved, an EOF error
        // is returned
        CERBERUS_EXPORT OpRes read(ByteBuffer& buf, LSIZE start = 0) const;

        // Read span buf from file starting from start pos
        CERBERUS_EXPORT OpRes read(ByteBuffer& buf, LSIZE start, LSIZE span) const;

        // Read a chunk of data from the current cursor position
        CERBERUS_EXPORT OpRes readChunk(ByteBuffer& buf, LSIZE chunksize) const;

        // Read a chunk of data from the current cursor position until a sequence is found.
        // The read sequence is inserted into the returned buffer
        CERBERUS_EXPORT OpResData<ByteBuffer> readUntil(const ByteBuffer& sequence) const;

        // Advance the cursor until the wanted sequence is found.
        // The position before this call is not reset, thus only the
        // bytes after the cursor position are searched.
        // Please note: this method will return the sequence starting
        // position but will leave the file cursor at the end of the sequence
        // (the byte after the sequence, or EOF)
        CERBERUS_EXPORT SizeOpRes search(const ByteBuffer& sequence) const;

        // Read a single line till \n or EOF
        // If the EOF is reached and the bytes read are zero, OR_EOF is returned
        // If an error occurs during read, OR_Failure is returned
        CERBERUS_EXPORT StringOpRes readLine() const;

        // Move the cursor to the absolute position pos
        CERBERUS_EXPORT OpRes seek(LSIZE pos) const;

        // Move the cursor back or forth according to the sign of the parameter pos
        CERBERUS_EXPORT OpRes seekOffset(OFFSET pos) const;

        // Move the cursor to the end of file
        CERBERUS_EXPORT OpRes seekToEOF() const;

        // Reset the cursor moving it to the beginning of the file
        CERBERUS_EXPORT void resetCursor() const;

        // Get the cursor position.
        CERBERUS_EXPORT SizeOpRes getCursor() const;

        // Check if this file and other file are equal (same size, same content)
        CERBERUS_EXPORT BoolOpRes isEqual(File& other) const;
    };
}  // namespace crb

#endif  // CERBERUS_DATA_FILESYSTEM_FILE_H
