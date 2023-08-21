#include "file.h"

#include <string.h>

#include <cstdio>
#include <regex>

#include "../../core/cerberusutils.h"
#include "src/core/cerberuslog.h"

#ifdef WINDOWS_SYSTEM
#include <windows.h>
#else
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

//=============================================================================
cerberus::OperationResult cerberus::data::filesystem::File::existsAsFile(const std::string& path)
{
    if (path.empty())
    {
        return OR_WrongArgument;
    }

#ifdef WINDOWS_SYSTEM

    if (GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES)  // TODO: true if is a file only
    {
        return true;
    }

#else
    struct stat stat_struct;
    int ret = stat(path.c_str(), &stat_struct);

    if (ret == 0)
    {
        if (S_ISREG(stat_struct.st_mode))
        {
            return true;
        }

        return false;
    }
    else
    {
        if (errno == ENOENT)
        {
            return false;
        }
        else
        {
            return OR_SystemFailure;
        }
    }

#endif
}
//=============================================================================
cerberus::OperationResult cerberus::data::filesystem::File::existsAsDirectory(const std::string& path)
{
#ifdef WINDOWS_SYSTEM
    throw cerberusImplementationMissExc("DIRECTORY EXISTANCE CHECK NOT IMPLEMENTED YET");

    if (GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES)  // TODO: true if is a directory only
    {
        //
    }

#else
    struct stat stat_struct;
    int ret = stat(path.c_str(), &stat_struct);

    if (ret == 0)
    {
        if (S_ISDIR(stat_struct.st_mode))
        {
            return true;
        }

        return false;
    }
    else
    {
        if (errno == ENOENT)
        {
            return false;
        }
        else
        {
            return OR_SystemFailure;
        }
    }

#endif
}
//=============================================================================
cerberus::OperationResult cerberus::data::filesystem::File::createDirectory(const std::string& path)
{
#ifdef WINDOWS_SYSTEM
    throw cerberusImplementationMissExc("DIRECTORY CREATION NOT IMPLEMENTED YET");
#else
    int ret = mkdir(path.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

    if (ret == -1)
    {
        logError("mkdir error: %s", strerror(errno));
        return OR_SystemFailure;
    }

#endif
    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::data::filesystem::File::deleteDirectory(const std::string& path)
{
#ifdef WINDOWS_SYSTEM
    throw cerberusImplementationMissExc("DIRECTORY DELETION NOT IMPLEMENTED YET");
#else
    int ret = rmdir(path.c_str());

    if (ret == -1)
    {
        logError("rmdir error: %s", strerror(errno));
        return OR_SystemFailure;
    }

#endif
    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::data::filesystem::File::isEmptyDirectory(const std::string& path)
{
#ifdef WINDOWS_SYSTEM
    throw cerberusImplementationMissExc("DIRECTORY EMPTY CHECK NOT IMPLEMENTED YET");
#else
    int n = 0;
    struct dirent* d;
    DIR* dir = opendir(path.c_str());

    if (dir == NULL)
    {
        return OR_InvalidPath;
    }

    errno = 0;

    while (readdir(dir) != NULL)
    {
        if (++n > 2)
        {
            break;
        }
    }

    closedir(dir);

    if (errno != 0)
    {
        logError("readdir error: %s", strerror(errno));
        return OR_SystemFailure;
    }

    if (n <= 2)
    {
        return true;
    }

    return false;
#endif
}
//=============================================================================
cerberus::OperationResult cerberus::data::filesystem::File::sizeOf(const std::string& path)
{
#ifdef WINDOWS_SYSTEM
    throw cerberusImplementationMissExc("sizeOf implementation missing");

#else
    struct stat stat_struct;
    int ret = stat(path.c_str(), &stat_struct);

    if (ret == 0)
    {
        SIZE s = stat_struct.st_size;
        return s;
    }
    else
    {
        return OR_Failure;
    }

#endif
}
//=============================================================================
cerberus::data::filesystem::File::File(FileOpenMode openMode, bool binaryMode)
    : m_filePath(),
      m_binaryMode(binaryMode),
      m_openMode(openMode),
      m_file(nullptr),
      m_fd(-1)
{
}
//=============================================================================
cerberus::data::filesystem::File::File(const std::string& filePath, FileOpenMode openMode, bool binaryMode)
    : m_filePath(filePath),
      m_binaryMode(binaryMode),
      m_openMode(openMode),
      m_file(nullptr),
      m_fd(-1)
{
}
//=============================================================================
cerberus::data::filesystem::File::~File() {}
//=============================================================================
bool cerberus::data::filesystem::File::setFileName(const std::string& filePath)
{
    if (isOpen())
    {
        return false;
    }

    m_filePath = filePath;

    return true;
}
//=============================================================================
bool cerberus::data::filesystem::File::canWrite() const { return (m_openMode != FOM_Read); }
//=============================================================================
std::string cerberus::data::filesystem::File::fileName() const { return m_filePath; }
//=============================================================================
bool cerberus::data::filesystem::File::setOpenMode(FileOpenMode openMode, bool binaryMode)
{
    if (isOpen())
    {
        return false;
    }

    m_openMode   = openMode;
    m_binaryMode = binaryMode;

    return true;
}
//=============================================================================
std::string cerberus::data::filesystem::File::getOpenModeString()
{
    std::string ret;

    switch (m_openMode)
    {
        case FOM_Read:
            ret = "r";
            break;
        case FOM_ReadWrite:
            ret = "r+";
            break;
        case FOM_ReadWriteTrunc:
            ret = "w+";
            break;
        case FOM_ReadWriteAppend:
            ret = "a+";
            break;
    }

    if (m_binaryMode)
    {
        ret += "b";
    }

    return ret;
}
//=============================================================================
bool cerberus::data::filesystem::File::isOpen() const { return m_file != NULL; }
//=============================================================================
bool cerberus::data::filesystem::File::open()
{
    if (isOpen() || m_filePath.empty())
    {
        return false;
    }

    m_file = fopen(m_filePath.c_str(), getOpenModeString().c_str());

    if (!isOpen())
    {
        return false;
    }

    m_fd = fileno(m_file);

    if (m_fd == -1)
    {
        return false;
    }

    return true;
}
//=============================================================================
bool cerberus::data::filesystem::File::close()
{
    if (fclose(m_file) != 0)
    {
        debug("fclose error: %s", strerror(errno));
        return false;
    }

    m_file = nullptr;
    m_fd   = -1;
    return true;
}
//=============================================================================
bool cerberus::data::filesystem::File::deleteFromDisk()
{
    if (isOpen())
    {
        return false;
    }

    if (::remove(m_filePath.c_str()) != 0)
    {
        debug("remove error: %s", strerror(errno));
        return false;
    }

    return true;
}
//=============================================================================
bool cerberus::data::filesystem::File::move(const std::string& newName)
{
    if (isOpen())
    {
        return false;
    }

    if (::rename(m_filePath.c_str(), newName.c_str()) != 0)
    {
        debug("rename error: %s", strerror(errno));
        return false;
    }

    m_filePath = newName;

    return true;
}
//=============================================================================
uint64_t cerberus::data::filesystem::File::size() const
{
    if (!isOpen())
    {
        return 0;
    }

    auto backup = ftell(m_file);

    auto ret = fseek(m_file, 0L, SEEK_END);

    if (ret == -1)
    {
        return 0;
    }

    auto size = ftell(m_file);

    ret = fseek(m_file, backup, SEEK_SET);

    if (ret == -1)
    {
        return 0;
    }

    return size;
}
//=============================================================================
bool cerberus::data::filesystem::File::write(const ByteBuffer& bytes)
{
    if (!isOpen())
    {
        return false;
    }

    if (fwrite(bytes.data(), 1, bytes.size(), m_file) != bytes.size())
    {
        return false;
    }

    fflush(m_file);
    return true;
}
//=============================================================================
bool cerberus::data::filesystem::File::writeLine(const std::string& line) { return write(core::CerberusUtils::strPrint("%s\n", line.c_str()).c_str()); }
//=============================================================================
bool cerberus::data::filesystem::File::read(ByteBuffer& bytes, uint64_t start) const
{
    if (!seek(start))
    {
        return false;
    }

    uint64_t bytesToRead = size() - start;

    bytes.resize(bytesToRead);

    clearerr(m_file);

    auto ret = fread(bytes.data(), 1, bytesToRead, m_file);

    if (ferror(m_file)) return false;

    if (feof(m_file) && !ret) return false;

    return true;
}
//=============================================================================
bool cerberus::data::filesystem::File::read(ByteBuffer& bytes, uint64_t start, uint64_t span) const
{
    if ((start + span) >= size())
    {
        return false;
    }

    if (!seek(start))
    {
        return false;
    }

    bytes.resize(span);

    clearerr(m_file);

    auto ret = fread(bytes.data(), 1, span, m_file);

    if (ferror(m_file)) return false;

    if (feof(m_file) && !ret) return false;

    return true;
}
//=============================================================================
bool cerberus::data::filesystem::File::readChunk(ByteBuffer& bytes, uint64_t chunksize) const
{
    bytes.clear();
    bytes.resize(chunksize);

    clearerr(m_file);

    auto ret = fread(bytes.data(), 1, chunksize, m_file);

    if (ferror(m_file)) return false;

    if (ret == 0 && feof(m_file))
    {
        return false;
    }

    bytes.resize(ret);

    return true;
}
//=============================================================================
bool cerberus::data::filesystem::File::readLine(std::string& line) const
{
    line.clear();

    clearerr(m_file);

    while (true)
    {
        char c = 0;

        fread(&c, 1, 1, m_file);

        if (ferror(m_file)) return false;

        if (feof(m_file) && line.empty()) return false;

        if (c == '\n' || feof(m_file))
        {
            break;
        }

        line += c;
    }

    return true;
}
//=============================================================================
bool cerberus::data::filesystem::File::seek(uint64_t pos) const
{
    if (!isOpen() || (pos >= size()))
    {
        return false;
    }

    auto ret = fseek(m_file, pos, SEEK_SET);

    if (ret == -1)
    {
        return false;
    }

    return true;
}
//=============================================================================
bool cerberus::data::filesystem::File::seekOffset(int64_t pos) const
{
    if (!isOpen())
    {
        return false;
    }

    clearerr(m_file);

    auto ret = fseek(m_file, pos, SEEK_CUR);

    if (ret == -1)
    {
        return false;
    }

    return true;
}
//=============================================================================
void cerberus::data::filesystem::File::resetCursor() const { ::rewind(m_file); }
//=============================================================================
uint64_t cerberus::data::filesystem::File::getCursor() const { return ftell(m_file); }
//=============================================================================
bool cerberus::data::filesystem::File::isEqual(File& other) const
{
    if (!isOpen() || !other.isOpen())
    {
        return false;
    }

    if (size() != other.size())
    {
        return false;
    }

    auto readBackup      = getCursor();
    auto readBackupOther = other.getCursor();

    resetCursor();
    other.resetCursor();

    ByteBuffer one;
    ByteBuffer two;
    bool equal = false;

    while (true)
    {
        bool read1 = readChunk(one, 50);
        bool read2 = other.readChunk(two, 50);

        if (read1 && read2)
        {
            if (one == two)
            {
                equal = true;
            }
            else
            {
                equal = false;
                break;
            }
        }
        else if (read1 != read2)
        {
            equal = false;
            break;
        }
        else
        {
            break;
        }
    }

    seek(readBackup);
    other.seek(readBackupOther);

    return equal;
}
//=============================================================================
