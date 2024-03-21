#include "file.h"

#include <string.h>

#include <cstdio>

#include "../../cerberus.h"
#include "../../core/cerberusutils.h"
#include "directory.h"

#ifdef WINDOWS_SYSTEM
#include <windows.h>
#else
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

using namespace cerberus;

//=============================================================================
OpRes File::existsAsFile(const std::string& path)
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
    int ret = ::stat(path.c_str(), &stat_struct);

    if (ret == 0)
    {
        if (S_ISREG(stat_struct.st_mode)) return OR_OK;

        return OR_InvalidPath;
    }
    else
    {
        if (errno == ENOENT) return OR_NotFound;

        return OR_SystemFailure;
    }

#endif
}
//=============================================================================
OpRes File::existsAsDirectory(const std::string& path)
{
#ifdef WINDOWS_SYSTEM
    throw cerberusImplementationMissExc("DIRECTORY EXISTANCE CHECK NOT IMPLEMENTED YET");

    if (GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES)  // TODO: true if is a directory only
    {
        //
    }

#else
    struct stat stat_struct;
    int ret = ::stat(path.c_str(), &stat_struct);

    if (ret == 0)
    {
        if (S_ISDIR(stat_struct.st_mode)) return OR_OK;

        return OR_InvalidPath;
    }
    else
    {
        if (errno == ENOENT) return OR_NotFound;

        return OR_SystemFailure;
    }

#endif
}
//=============================================================================
::cerberus::OpResData<LSIZE> File::directorySize(const std::string& path)
{
    // Directory dir(path);
    // LSIZE size = 0;

    // auto files = dir.listFiles();
    // auto dirs  = dir.listDirs();

    // for (auto& el : files)
    // {
    //     size += el.size().value;
    // }

    // for (auto& el : dirs)
    // {
    //     size += el.size().value;
    // }

    // return size;
    return OR_OK;
}
//=============================================================================
OpRes File::createDirectory(const std::string& path)
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
OpRes File::remove(const std::string& path)
{
#ifdef WINDOWS_SYSTEM
    throw cerberusImplementationMissExc("REMOVE NOT IMPLEMENTED YET");
#else
    if (::remove(path.c_str()) == -1) return {OR_Failure, strerror(errno)};

    return OR_OK;
#endif
}
//=============================================================================
OpRes File::move(const std::string& oldPath, const std::string& newPath)
{
#ifdef WINDOWS_SYSTEM
    throw cerberusImplementationMissExc("MOVE NOT IMPLEMENTED YET");
#else
    if (::rename(oldPath.c_str(), newPath.c_str()) == -1) return {OR_Failure, strerror(errno)};

    return OR_OK;
#endif
}
//=============================================================================
OpRes File::isEmptyDirectory(const std::string& path)
{
#ifdef WINDOWS_SYSTEM
    throw cerberusImplementationMissExc("DIRECTORY EMPTY CHECK NOT IMPLEMENTED YET");
#else
    int n = 0;
    struct dirent* d;
    DIR* dir = opendir(path.c_str());

    if (dir == NULL) return OR_InvalidPath;

    errno = 0;

    while (readdir(dir) != NULL)
    {
        if (++n > 2) break;
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
OpResData<FileMetadata> cerberus::File::stat(const std::string& path)
{
#ifdef WINDOWS_SYSTEM
    throw cerberusImplementationMissExc("stat implementation missing");

#else
    struct stat stat_struct = {};

    int ret = ::stat(path.c_str(), &stat_struct);

    if (ret != 0) return {OR_Failure, strerror(errno)};

    FileMetadata metadata = {};

    metadata.fromStat(stat_struct);

    return metadata;
#endif
}
//=============================================================================
File::File(FileOpenMode openMode, bool binaryMode)
    : m_name(),
      m_binaryMode(binaryMode),
      m_openMode(openMode),
      m_file(nullptr),
      m_fd(-1)
{
}
//=============================================================================
File::File(const std::string& path, FileOpenMode openMode, bool binaryMode)
    : m_name(),
      m_binaryMode(binaryMode),
      m_openMode(openMode),
      m_file(nullptr),
      m_fd(-1)
{
    setPath(path);
}
//=============================================================================
File::~File() {}
//=============================================================================
void File::path(const std::string& path) { setPath(path); }
//=============================================================================
bool File::canWrite() const { return (m_openMode != FOM_Read); }
//=============================================================================
std::string File::name() const { return m_name; }
//=============================================================================
std::string File::path() const { return m_parent.copy_append(m_name).toStr(); }
//=============================================================================
std::string File::completePath() const { return CerberusUtils::completePath(path()).value; }
//=============================================================================
void File::setOpenMode(FileOpenMode openMode, bool binaryMode)
{
    m_openMode   = openMode;
    m_binaryMode = binaryMode;
}
//=============================================================================
std::string File::getOpenModeString()
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
void File::setPath(const std::string& path)
{
    m_parent.fromStr(path);
    m_name = m_parent.back();
    if (!m_parent.empty()) m_parent.pop_back();
}
//=============================================================================
File::File(Path parent, const std::string& name)
    : m_name(name),
      m_binaryMode(false),
      m_openMode(FOM_Read),
      m_file(nullptr),
      m_fd(-1),
      m_parent(parent)
{
}
//=============================================================================
bool File::isOpen() const { return m_file != NULL; }
//=============================================================================
OpRes File::open()
{
    if (m_name.empty()) return OR_InvalidPath;

    m_file = fopen(path().c_str(), getOpenModeString().c_str());

    if (!isOpen()) return {OR_Failure, strerror(errno)};

    m_fd = fileno(m_file);

    if (m_fd == -1) throw cerberusSystemExc("stream is not associated with a file");

    return OR_OK;
}
//=============================================================================
void File::close()
{
    if (!isOpen()) return;

    if (fclose(m_file) != 0) logError("fclose error: %s", strerror(errno));

    m_file = nullptr;
    m_fd   = -1;
}
//=============================================================================
OpRes File::deleteFromDisk()
{
    if (isOpen()) close();

    return remove(path());
}
//=============================================================================
OpRes File::move(const std::string& newPath)
{
    auto res = move(path(), newPath);

    if (res.ok()) setPath(newPath);

    return res;
}
//=============================================================================
OpResData<FileMetadata> File::metadata()
{
    if (!isOpen()) return OR_BadConditions;

    struct stat stat_struct = {};

    int ret = fstat(m_fd, &stat_struct);

    if (ret != 0) return {OR_Failure, strerror(errno)};

    FileMetadata metadata = {};

    metadata.fromStat(stat_struct);

    return metadata;
}
//=============================================================================
SizeOpRes File::size() const
{
    if (!isOpen())
    {
        auto st = File::stat(path());
        if (st.fail()) return st;

        return st.value.size;
    }

    auto backup = ftell(m_file);

    auto ret = fseek(m_file, 0L, SEEK_END);

    if (ret == -1) return OR_Failure;

    auto size = ftell(m_file);

    ret = fseek(m_file, backup, SEEK_SET);

    if (ret == -1) return OR_Failure;

    return (LSIZE)size;
}
//=============================================================================
OpRes File::write(const ByteBuffer& bytes)
{
    if (!isOpen()) return OR_BadConditions;

    if (fwrite(bytes.data(), 1, bytes.size(), m_file) != bytes.size()) return OR_Failure;

    fflush(m_file);

    return OR_OK;
}
//=============================================================================
OpRes File::writeLine(const std::string& line)
{
    return write(CerberusUtils::strPrint("%s\n", line.c_str()).c_str());
}
//=============================================================================
OpRes File::read(ByteBuffer& bytes, LSIZE start) const
{
    if (!isOpen()) return OR_BadConditions;

    auto res = seek(start);

    if (res.fail()) return res;

    uint64_t bytesToRead = size().value - start;

    bytes.resize(bytesToRead);

    clearerr(m_file);

    auto ret = fread(bytes.data(), 1, bytesToRead, m_file);

    if (ferror(m_file)) return OR_Failure;

    if (feof(m_file) && !ret) return OR_EOF;

    return OR_OK;
}
//=============================================================================
OpRes File::read(ByteBuffer& bytes, LSIZE start, LSIZE span) const
{
    if (!isOpen()) return OR_BadConditions;

    if ((start + span) >= size().value) return OR_WrongArgument;

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
OpRes File::readChunk(ByteBuffer& bytes, SIZE chunksize) const
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
StringOpRes File::readLine() const
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

    return line;
}
//=============================================================================
OpRes File::seek(cerberus::LSIZE pos) const
{
    if (!isOpen()) return OR_BadConditions;

    if (pos >= size().value) return OR_WrongArgument;

    auto ret = fseek(m_file, pos, SEEK_SET);

    if (ret == -1) return OR_Failure;

    return OR_OK;
}
//=============================================================================
OpRes File::seekOffset(int64_t pos) const
{
    if (!isOpen()) return OR_BadConditions;

    clearerr(m_file);

    if (fseek(m_file, pos, SEEK_CUR) == -1) return {OR_Failure, strerror(errno)};

    return OR_OK;
}
//=============================================================================
void File::resetCursor() const { ::rewind(m_file); }
//=============================================================================
SizeOpRes File::getCursor() const
{
    if (!isOpen()) return OR_BadConditions;

    auto pos = ftell(m_file);

    if (pos == -1) return {OR_Failure, strerror(errno)};

    return (LSIZE)pos;
}
//=============================================================================
BoolOpRes File::isEqual(File& other) const
{
    if (!isOpen() || !other.isOpen()) return OR_BadConditions;

    auto r1 = size();
    auto r2 = other.size();
    if (r1.fail()) return r1;
    if (r2.fail()) return r2;

    if (r1.value != r2.value) return false;

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

    seek(c1.value);
    other.seek(c2.value);

    return equal;
}
//=============================================================================
