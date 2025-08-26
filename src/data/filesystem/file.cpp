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

#define MAXIMUM_COPY_BLOCKSIZE 4096u  // 4k block size

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

    if (errno != 0)
    {
        logError("readdir error: %s", strerror(errno));
        closedir(dir);
        return OR_SystemFailure;
    }

    closedir(dir);

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
    FileMetadata metadata = {};

#if defined(WINDOWS_SYSTEM)
    throw cerberusImplementationMissExc("stat implementation missing");

#elif defined(LINUX_SYSTEM)
    struct statx stat_struct = {};

    int flags = 0;
    if (path.empty()) flags |= AT_EMPTY_PATH;

    unsigned int mask = STATX_BASIC_STATS | STATX_BTIME;

    int ret = ::statx(AT_FDCWD, path.c_str(), flags, mask, &stat_struct);

    if (ret == -1) return {OR_Failure, "statx error", strerror(errno)};

    if ((mask & stat_struct.stx_mask) != mask) return {OR_Failure, "incomplete data from statx"};

#elif defined(APPLE_SYSTEM)

    struct stat stat_struct = {};

    int ret = ::stat(path.c_str(), &stat_struct);

    if (ret == -1) return {OR_Failure, "stat error", strerror(errno)};

#endif

    metadata.fromStat(stat_struct);
    return metadata;
}
//=============================================================================
File File::tmpFile(const Path& path, FileOpenMode openMode)
{
    Path pth = path;
    if (pth.empty()) pth.fromStr(P_tmpdir);

    pth.append("XXXXXX");
    std::string pstring(pth.toStr());

    // this will overwrite 'XXXXXX'
    int newfd = ::mkostemp(pstring.data(), openMode == FOM_ReadWriteAppend ? O_APPEND : 0);

    if (newfd == -1) throw cSystemExc("mkstemp error: %s", strerror(errno));

    return std::move(File(newfd, Path(pstring), openMode));
}
//=============================================================================
OpRes File::zeroCopy(File& src, File& dst, LSIZE len)
{
#if defined(WINDOWS_SYSTEM)
    throw cImplMissExc("zerocopy implementation missing");

#elif defined(LINUX_SYSTEM)
    throw cImplMissExc("zerocopy implementation missing");

#elif defined(APPLE_SYSTEM)
    // user-space copy for macos, sadly..

    LSIZE blocksize = MAXIMUM_COPY_BLOCKSIZE;
    if (len && len < blocksize) blocksize = len;
    ByteBuffer buf;

    while (true)  // maybe we can put this code in a userCopy() static function
    {
        auto r = src.readChunk(buf, blocksize);
        if (r.res == OR_Failure) return r;

        if (r.res == OR_EOF) break;

        condret(dst.write(buf));

        if (!len) continue;  // if len == 0 do not count

        len -= blocksize;
        if (len < blocksize) blocksize = len;

        if (len == 0) break;
    }

#endif

    return OR_OK;
}
//=============================================================================
File::File(FileOpenMode openMode)
    : m_path(),
      m_openMode(openMode),
      m_file(nullptr),
      m_fd(-1)
{
}
//=============================================================================
File::File(const Path& path, FileOpenMode openMode)
    : m_path(path),
      m_openMode(openMode),
      m_file(nullptr),
      m_fd(-1)
{
}
//=============================================================================
File::File(const File& other)
    : m_path(other.m_path),
      m_openMode(other.m_openMode),
      m_file(nullptr),
      m_fd(-1)
{
}
//=============================================================================
File::File(File&& other)
    : m_path(other.m_path),
      m_openMode(other.m_openMode),
      m_file(other.m_file),
      m_fd(other.m_fd)
{
    other.m_fd   = -1;
    other.m_file = nullptr;
}
//=============================================================================
File& File::operator=(const File& other)
{
    m_path     = other.m_path;
    m_openMode = other.m_openMode;
    return *this;
}
//=============================================================================
File::~File()
{
    if (isOpen()) close();
}
//=============================================================================
OpResData<FileMetadata> File::stat()
{
    if (!isOpen()) return File::stat(m_path.toStr());

    FileMetadata metadata = {};

#if defined(LINUX_SYSTEM)

    struct statx stat_struct = {};

    unsigned int mask = STATX_BASIC_STATS | STATX_BTIME;

    int ret = ::statx(m_fd, "", AT_EMPTY_PATH, mask, &stat_struct);

    if (ret == -1) return {OR_Failure, "statx error", strerror(errno)};

    if ((mask & stat_struct.stx_mask) != mask) return {OR_Failure, "incomplete data from statx"};

#elif defined(APPLE_SYSTEM)

    struct stat stat_struct = {};

    int ret = ::fstat(m_fd, &stat_struct);

    if (ret == -1) return {OR_Failure, "fstat error", strerror(errno)};

#endif

    metadata.fromStat(stat_struct);
    return metadata;
}
//=============================================================================
bool File::canWrite() const { return (m_openMode != FOM_Read); }
//=============================================================================
std::string File::name() const { return (m_path.empty() ? "" : m_path.back()); }
//=============================================================================
Path File::path() const { return m_path; }
//=============================================================================
Path File::completePath() const { return CerberusUtils::completePath(m_path.toStr()).value; }
//=============================================================================
Path File::directory() const
{
    auto path = completePath();
    path.pop_back();
    return path;
}
//=============================================================================
void File::setOpenMode(FileOpenMode openMode)
{
    if (isOpen()) throw cIllegalStateExc("cannot alter the open mode of an open file");
    m_openMode = openMode;
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

    ret += "b";

    return ret;
}
//=============================================================================
OpRes File::_seek(LSIZE pos) const
{
    if (fseeko(m_file, (off_t)pos, SEEK_SET) == -1)
        return {OR_Failure, CerberusUtils::strPrint("fseek fail: %s", strerror(errno))};

    return OR_OK;
}
//=============================================================================
File::File(int fd, const Path& path, FileOpenMode openMode)
    : m_path(path),
      m_openMode(openMode),
      m_file(NULL),
      m_fd(fd)
{
    m_file = ::fdopen(m_fd, getOpenModeString().c_str());

    if (m_file == NULL)
    {
        auto err = errno;
        ::close(m_fd);  // RAII
        throw cSystemExc("fdopen error: %s", strerror(err));
    }

    resetCursor();  // fdopen does not set cursor to zero
}
//=============================================================================
void File::path(const Path& path) { m_path = path; }
//=============================================================================
bool File::isOpen() const { return m_file != NULL; }
//=============================================================================
OpRes File::open()
{
    if (m_path.empty()) return OR_InvalidPath;

    m_file = fopen(m_path.toStr().c_str(), getOpenModeString().c_str());

    if (!isOpen()) return {OR_Failure, strerror(errno)};

    m_fd = fileno(m_file);

    if (m_fd == -1) throw cSystemExc("stream is not associated with a file");

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
OpRes File::reopen()
{
    File::close();
    return File::open();
}
//=============================================================================
OpRes File::remove()
{
    if (isOpen()) close();

    return remove(m_path.toStr());
}
//=============================================================================
OpRes File::move(const Path& newPath)
{
    auto res = move(m_path.toStr(), newPath.toStr());

    if (res.ok()) path(newPath);

    return res;
}
//=============================================================================
SizeOpRes File::size() const
{
    if (!isOpen())
    {
        auto st = File::stat(m_path.toStr());
        if (st.fail()) return st;

        return st.value.size;
    }

    auto backup = getCursor().expect().value;
    condret(seekToEOF());
    auto size = getCursor().expect().value;
    condret(_seek(backup));
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
OpRes File::writeExpand(const ByteBuffer& bytes)
{
    if (!isOpen()) return OR_BadConditions;

    FileOpenMode fombackup = m_openMode;
    LSIZE cursorbackup     = getCursor().expect().value;

    if (m_openMode != FOM_ReadWriteAppend)
    {
        // reopen for appending
        close();

        setOpenMode(FOM_ReadWriteAppend);
        condret(open());

        write(bytes);
        close();

        setOpenMode(fombackup);
        condret(open());
        condret(seek(cursorbackup));
        return OR_OK;
    }

    write(bytes);
    return OR_OK;
}
//=============================================================================
OpRes File::insert(const ByteBuffer& bytes)
{
    if (!isOpen()) return OR_BadConditions;

    auto dir = directory();
    auto tmp = tmpFile(dir, FOM_ReadWriteAppend);  // the file will be enlarged

    LSIZE inspoint = getCursor().expect().value;

    resetCursor();  // now position is 0

    condret(zeroCopy(*this, tmp, inspoint));  // copy inspoint bytes
    condret(tmp.write(bytes));                // add the buffer
    condret(zeroCopy(*this, tmp));            // copy remaining bytes

    // overwrite this file with the temporary one
    condret(File::move(tmp.completePath().toStr(), completePath().toStr()));

    condret(reopen());

    return OR_OK;
}
//=============================================================================
OpRes File::read(ByteBuffer& bytes, LSIZE start) const
{
    if (!isOpen()) return OR_BadConditions;

    auto res = seek(start);

    if (res.fail()) return res;

    LSIZE bytesToRead = size().value - start;

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

    bytes.resize(ret);

    if (ferror(m_file)) return OR_Failure;

    if (ret == 0 && feof(m_file)) return OR_EOF;

    return OR_OK;
}
//=============================================================================
OpResData<ByteBuffer> File::readUntil(const ByteBuffer& sequence) const
{
    auto backup = getCursor();
    auto seqpos = search(sequence);
    seek(backup.value).expect();

    if (seqpos.fail()) return seqpos;

    ByteBuffer buf;
    auto readret = readChunk(buf, seqpos.value - backup.value);
    if (readret.fail()) return readret;

    return buf;
}
//=============================================================================
SizeOpRes File::search(const ByteBuffer& sequence) const
{
    if (sequence.isEmpty()) return OR_WrongArgument;
    if (!isOpen()) return OR_BadConditions;
    if (feof(m_file)) return OR_EOF;

    clearerr(m_file);
    char c          = 0;
    size_t seqindex = 0;
    size_t seqlast  = sequence.size() - 1;

    while (true)
    {
        fread(&c, 1, 1, m_file);

        if (ferror(m_file)) return OR_Failure;
        if (feof(m_file)) return OR_NotFound;

        if (c == sequence[seqindex])
        {
            if (seqindex == seqlast) return getCursor().expect().value - sequence.size();
            seqindex++;
        }
        else if (seqindex)
        {
            seekOffset(-1).expect();
            seqindex = 0;
        }
    }
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

    if (pos > size().expect().value) return OR_WrongArgument;

    return _seek(pos);
}
//=============================================================================
OpRes File::seekOffset(int64_t pos) const
{
    if (!isOpen()) return OR_BadConditions;

    clearerr(m_file);

    if (fseeko(m_file, (off_t)pos, SEEK_CUR) == -1) return {OR_Failure, strerror(errno)};

    return OR_OK;
}
//=============================================================================
OpRes File::seekToEOF() const
{
    if (!isOpen()) return OR_BadConditions;

    clearerr(m_file);

    if (fseeko(m_file, (off_t)0, SEEK_END) == -1) return {OR_Failure, strerror(errno)};

    return OR_OK;
}
//=============================================================================
void File::resetCursor() const { ::rewind(m_file); }
//=============================================================================
SizeOpRes File::getCursor() const
{
    if (!isOpen()) return OR_BadConditions;

    auto pos = ftello(m_file);
    if (pos == (off_t)-1) return {OR_Failure, strerror(errno)};
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
