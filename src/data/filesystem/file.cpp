#include "file.h"

#include <string.h>

#include <cstdio>

#include "../../cerberus.h"
#include "../../core/cerberusutils.h"

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
            return OR_OK;
        }

        return OR_InvalidPath;
    }
    else
    {
        if (errno == ENOENT)
        {
            return OR_InvalidPath;
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
            return OR_OK;
        }

        return OR_InvalidPath;
    }
    else
    {
        if (errno == ENOENT)
        {
            return OR_InvalidPath;
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
cerberus::OperationResult cerberus::data::filesystem::File::remove(const std::string& path)
{
#ifdef WINDOWS_SYSTEM
    throw cerberusImplementationMissExc("REMOVE NOT IMPLEMENTED YET");
#else
    if (::remove(path.c_str()) == -1)
    {
        return {OR_Failure, strerror(errno)};
    }

    return OR_OK;
#endif
}
//=============================================================================
cerberus::OperationResult cerberus::data::filesystem::File::move(const std::string& oldPath, const std::string& newPath)
{
#ifdef WINDOWS_SYSTEM
    throw cerberusImplementationMissExc("MOVE NOT IMPLEMENTED YET");
#else
    if (::rename(oldPath.c_str(), newPath.c_str()) == -1)
    {
        return {OR_Failure, strerror(errno)};
    }

    return OR_OK;
#endif
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
        return OR_OK;
    }

    return OR_NotEmpty;
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
        LSIZE s = stat_struct.st_size;
        return OperationResult((LSIZE)s);
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
void cerberus::data::filesystem::File::setFileName(const std::string& filePath) { m_filePath = filePath; }
//=============================================================================
bool cerberus::data::filesystem::File::canWrite() const { return (m_openMode != FOM_Read); }
//=============================================================================
std::string cerberus::data::filesystem::File::fileName() const { return m_filePath; }
//=============================================================================
void cerberus::data::filesystem::File::setOpenMode(FileOpenMode openMode, bool binaryMode)
{
    m_openMode   = openMode;
    m_binaryMode = binaryMode;
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

    if (m_binaryMode) ret += "b";

    return ret;
}
//=============================================================================
bool cerberus::data::filesystem::File::isOpen() const { return m_file != NULL; }
//=============================================================================
cerberus::OperationResult cerberus::data::filesystem::File::open()
{
    if (m_filePath.empty()) return OR_InvalidPath;

    m_file = fopen(m_filePath.c_str(), getOpenModeString().c_str());

    if (!isOpen()) return {OR_Failure, strerror(errno)};

    m_fd = fileno(m_file);

    if (m_fd == -1) throw cerberusSystemExc("stream is not associated with a file");

    return OR_OK;
}
//=============================================================================
void cerberus::data::filesystem::File::close()
{
    if (!isOpen()) return;

    if (fclose(m_file) != 0) logError("fclose error: %s", strerror(errno));

    m_file = nullptr;
    m_fd   = -1;
}
//=============================================================================
cerberus::OperationResult cerberus::data::filesystem::File::deleteFromDisk()
{
    if (isOpen())
    {
        close();
    }

    return remove(m_filePath);
}
//=============================================================================
cerberus::OperationResult cerberus::data::filesystem::File::move(const std::string& newName)
{
    auto res = move(m_filePath, newName);

    if (res.ok()) m_filePath = newName;

    return res;
}
//=============================================================================
cerberus::OperationResult cerberus::data::filesystem::File::size() const
{
    // maybe we should use fstat() ?

    if (!isOpen()) return OR_BadConditions;

    auto backup = ftell(m_file);

    auto ret = fseek(m_file, 0L, SEEK_END);

    if (ret == -1) return OR_Failure;

    auto size = ftell(m_file);

    ret = fseek(m_file, backup, SEEK_SET);

    if (ret == -1) return OR_Failure;

    return OperationResult((LSIZE)size);
}
//=============================================================================
cerberus::OperationResult cerberus::data::filesystem::File::write(const ByteBuffer& bytes)
{
    if (!isOpen()) return OR_BadConditions;

    if (fwrite(bytes.data(), 1, bytes.size(), m_file) != bytes.size()) return OR_Failure;

    fflush(m_file);

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::data::filesystem::File::writeLine(const std::string& line) { return write(core::CerberusUtils::strPrint("%s\n", line.c_str()).c_str()); }
//=============================================================================
cerberus::OperationResult cerberus::data::filesystem::File::read(ByteBuffer& bytes, LSIZE start) const
{
    if (!isOpen()) return OR_BadConditions;

    auto res = seek(start);

    if (res.fail()) return res;

    uint64_t bytesToRead = size().sz - start;

    bytes.resize(bytesToRead);

    clearerr(m_file);

    auto ret = fread(bytes.data(), 1, bytesToRead, m_file);

    if (ferror(m_file)) return OR_Failure;

    if (feof(m_file) && !ret) return OR_EOF;

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::data::filesystem::File::read(ByteBuffer& bytes, LSIZE start, LSIZE span) const
{
    if (!isOpen()) return OR_BadConditions;

    if ((start + span) >= size().sz) return OR_WrongArgument;

    auto res = seek(start);

    if (res.fail()) return res;

    bytes.resize(span);

    clearerr(m_file);

    auto ret = fread(bytes.data(), 1, span, m_file);

    if (ferror(m_file)) return OR_Failure;

    if (feof(m_file) && !ret) return OR_EOF;

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::data::filesystem::File::readChunk(ByteBuffer& bytes, SIZE chunksize) const
{
    if (!isOpen()) return OR_BadConditions;

    bytes.clear();
    bytes.resize(chunksize);

    clearerr(m_file);

    auto ret = fread(bytes.data(), 1, chunksize, m_file);

    if (ferror(m_file)) return OR_Failure;

    if (ret == 0 && feof(m_file)) return OR_EOF;

    bytes.resize(ret);

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::data::filesystem::File::readLine() const
{
    if (!isOpen()) return OR_BadConditions;

    clearerr(m_file);
    std::string line;

    while (true)
    {
        char c = 0;

        fread(&c, 1, 1, m_file);

        if (ferror(m_file)) return OR_Failure;

        if (feof(m_file) && line.empty()) return OR_EOF;

        if (c == '\n' || feof(m_file)) break;

        line += c;
    }

    return OperationResult(line);
}
//=============================================================================
cerberus::OperationResult cerberus::data::filesystem::File::seek(cerberus::LSIZE pos) const
{
    if (!isOpen()) return OR_BadConditions;

    if (pos >= size().sz) return OR_WrongArgument;

    auto ret = fseek(m_file, pos, SEEK_SET);

    if (ret == -1) return OR_Failure;

    return OR_OK;
}
//=============================================================================
cerberus::OperationResult cerberus::data::filesystem::File::seekOffset(int64_t pos) const
{
    if (!isOpen()) return OR_BadConditions;

    clearerr(m_file);

    if (fseek(m_file, pos, SEEK_CUR) == -1) return {OR_Failure, strerror(errno)};

    return OR_OK;
}
//=============================================================================
void cerberus::data::filesystem::File::resetCursor() const { ::rewind(m_file); }
//=============================================================================
cerberus::OperationResult cerberus::data::filesystem::File::getCursor() const
{
    if (!isOpen()) return OR_BadConditions;

    auto pos = ftell(m_file);

    if (pos == -1) return {OR_Failure, strerror(errno)};

    return OperationResult((LSIZE)pos);
}
//=============================================================================
cerberus::OperationResult cerberus::data::filesystem::File::isEqual(File& other) const
{
    if (!isOpen() || !other.isOpen()) return OR_BadConditions;

    auto r1 = size();
    auto r2 = other.size();
    if (r1.fail()) return r1;
    if (r2.fail()) return r2;

    if (r1.sz != r2.sz) return (int64_t)0;

    auto c1 = getCursor();
    auto c2 = other.getCursor();
    if (c1.fail()) return c1;
    if (c2.fail()) return c2;

    resetCursor();
    other.resetCursor();

    ByteBuffer first;
    ByteBuffer second;
    bool equal = false;

    while (true)
    {
        auto read1 = readChunk(first, 50);
        auto read2 = other.readChunk(second, 50);

        if (read1.ok() && read2.ok())
        {
            if (first == second)
            {
                equal = true;
            }
            else
            {
                equal = false;
                break;
            }
        }
        else if (read1.res != read2.res)
        {
            equal = false;
            break;
        }
        else
        {
            break;
        }
    }

    seek(c1.sz);
    other.seek(c2.sz);

    return (int64_t)equal;
}
//=============================================================================
