#include "file.h"

#include <string.h>

#include <cstdio>

#include "../../cerberus.h"
#include "../../core/cerberusutils.h"

#ifdef WINDOWS_SYSTEM
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <windows.h>
#include <shlwapi.h>
#define fseeko _fseeki64
#define ftello _ftelli64
#define fdopen _fdopen
#define fileno _fileno
#define O_APPEND _O_APPEND
#define O_CREAT _O_CREAT
#define O_EXCL _O_EXCL
#define O_RDWR _O_RDWR
#define O_BINARY _O_BINARY
#define S_IREAD _S_IREAD
#define S_IWRITE _S_IWRITE
#else
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#define MAXIMUM_COPY_BLOCKSIZE 4096u  // 4k block size

using namespace crb;

#ifdef WINDOWS_SYSTEM
namespace
{
DateTime fileTimeToDateTime(const FILETIME& ft)
{
    ULARGE_INTEGER value{};
    value.LowPart  = ft.dwLowDateTime;
    value.HighPart = ft.dwHighDateTime;

    constexpr uint64_t windowsToUnixEpoch100ns = 116444736000000000ULL;
    if (value.QuadPart < windowsToUnixEpoch100ns) return DateTime();

    const uint64_t unix100ns = value.QuadPart - windowsToUnixEpoch100ns;
    DateTime ret;
    ret.fromTimespec(static_cast<int64_t>(unix100ns / 10000000ULL),
                     static_cast<int64_t>((unix100ns % 10000000ULL) * 100ULL));
    return ret;
}

FileMetadata fileMetadataFromWin32(const WIN32_FILE_ATTRIBUTE_DATA& stat_struct)
{
    FileMetadata metadata = {};
    metadata.type = (stat_struct.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? FT_DIR : FT_REG;

    metadata.linkrefs = 1;
    metadata.size     = (static_cast<LSIZE>(stat_struct.nFileSizeHigh) << 32) | stat_struct.nFileSizeLow;
    metadata.ownUID   = 0;
    metadata.ownGID   = 0;

    metadata.mode.user  = FP_READ;
    metadata.mode.group = FP_READ;
    metadata.mode.other = FP_READ;
    if (!(stat_struct.dwFileAttributes & FILE_ATTRIBUTE_READONLY))
    {
        metadata.mode.user |= FP_WRITE;
        metadata.mode.group |= FP_WRITE;
        metadata.mode.other |= FP_WRITE;
    }

    metadata.accTime = fileTimeToDateTime(stat_struct.ftLastAccessTime);
    metadata.modTime = fileTimeToDateTime(stat_struct.ftLastWriteTime);
    metadata.chgTime = metadata.modTime;
    metadata.creTime = fileTimeToDateTime(stat_struct.ftCreationTime);
    return metadata;
}
}  // namespace
#endif

//=============================================================================
OpRes File::existsAsFile(const std::string& path)
{
    if (path.empty())
    {
        return OR_WrongArgument;
    }

#ifdef WINDOWS_SYSTEM
    DWORD attr = GetFileAttributesA(path.c_str());

    if (attr != INVALID_FILE_ATTRIBUTES)
    {
        if (!(attr & FILE_ATTRIBUTE_DIRECTORY)) return OR_OK;

        return OR_InvalidPath;
    }

    if (GetLastError() == ERROR_FILE_NOT_FOUND || GetLastError() == ERROR_PATH_NOT_FOUND) return OR_NotFound;

    return OR_SystemFailure;
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
    DWORD attr = GetFileAttributesA(path.c_str());

    if (attr != INVALID_FILE_ATTRIBUTES)
    {
        if (attr & FILE_ATTRIBUTE_DIRECTORY) return OR_OK;

        return OR_InvalidPath;
    }

    if (GetLastError() == ERROR_FILE_NOT_FOUND || GetLastError() == ERROR_PATH_NOT_FOUND) return OR_NotFound;

    return OR_SystemFailure;
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

    if (CreateDirectoryA(path.c_str(), NULL) == 0)
    {
        DWORD err = GetLastError();
        if (err == ERROR_ALREADY_EXISTS) return OR_AlreadyPresent;
        if (err == ERROR_PATH_NOT_FOUND) return OR_InvalidPath;

        return OR_SystemFailure;
    }

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
    DWORD attr = GetFileAttributesA(path.c_str());

    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND || GetLastError() == ERROR_PATH_NOT_FOUND) return OR_NotFound;

        return OR_SystemFailure;
    }

    BOOL ok = (attr & FILE_ATTRIBUTE_DIRECTORY) ? RemoveDirectoryA(path.c_str()) : DeleteFileA(path.c_str());
    if (ok == 0) return OR_SystemFailure;

    return OR_OK;
#else
    if (::remove(path.c_str()) == -1) return {OR_Failure, strerror(errno)};

    return OR_OK;
#endif
}
//=============================================================================
OpRes File::move(const std::string& oldPath, const std::string& newPath)
{
#ifdef WINDOWS_SYSTEM
    if (MoveFileExA(oldPath.c_str(), newPath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED) == 0)
    {
        DWORD err = GetLastError();
        return {OR_SystemFailure,
                CerberusUtils::strPrint("MoveFileExA failed from '%s' to '%s' with error %lu",
                                        oldPath.c_str(), newPath.c_str(),
                                        static_cast<unsigned long>(err))};
    }

    return OR_OK;
#else
    if (::rename(oldPath.c_str(), newPath.c_str()) == -1) return {OR_Failure, strerror(errno)};

    return OR_OK;
#endif
}
//=============================================================================
OpRes File::isEmptyDirectory(const std::string& path)
{
#ifdef WINDOWS_SYSTEM

    OpRes dir = existsAsDirectory(path);
    if (dir.fail()) return dir;

    if (PathIsDirectoryEmptyA(path.c_str()) == TRUE) return OR_OK;

    return OR_NotEmpty;

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
OpResData<FileMetadata> crb::File::stat(const std::string& path)
{
#if defined(WINDOWS_SYSTEM)
    WIN32_FILE_ATTRIBUTE_DATA stat_struct = {};

    if (GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &stat_struct) == 0)
    {
        DWORD err = GetLastError();
        if (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND) return OR_NotFound;
        return OR_SystemFailure;
    }

    return fileMetadataFromWin32(stat_struct);

#else
    FileMetadata metadata = {};

#if defined(LINUX_SYSTEM)
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
#endif
}
//=============================================================================
File File::tmpFile(const Path& path, FileOpenMode openMode)
{
    Path pth = path;
#ifdef WINDOWS_SYSTEM
    char tempDir[MAX_PATH + 1] = {};
    if (pth.empty())
    {
        DWORD len = GetTempPathA(MAX_PATH, tempDir);
        if (len == 0 || len > MAX_PATH) throw cSystemExc("GetTempPathA error");
    }
    else
    {
        std::string dir = pth.toStr();
        DWORD len       = GetFullPathNameA(dir.c_str(), MAX_PATH, tempDir, nullptr);
        if (len == 0 || len > MAX_PATH) throw cSystemExc("GetFullPathNameA error");
    }

    char tempPath[MAX_PATH + 1] = {};
    if (GetTempFileNameA(tempDir, "crb", 0, tempPath) == 0) throw cSystemExc("GetTempFileNameA error");

    int flags = O_RDWR | O_BINARY;
    if (openMode == FOM_ReadWriteAppend) flags |= O_APPEND;

    int newfd = ::_open(tempPath, flags, S_IREAD | S_IWRITE);
    if (newfd == -1) throw cSystemExc("open temp file error: %s", strerror(errno));

    return std::move(File(newfd, Path(tempPath), openMode));
#else
    if (pth.empty()) pth.fromStr(P_tmpdir);

    pth.append("XXXXXX");
    std::string pstring(pth.toStr());

    // this will overwrite 'XXXXXX'
    int newfd = ::mkostemp(pstring.data(), openMode == FOM_ReadWriteAppend ? O_APPEND : 0);

    if (newfd == -1) throw cSystemExc("mkstemp error: %s", strerror(errno));

    return std::move(File(newfd, Path(pstring), openMode));
#endif
}
//=============================================================================
OpRes File::zeroCopy(File& src, File& dst, LSIZE len)
{
#if defined(WINDOWS_SYSTEM)
    LSIZE blocksize = MAXIMUM_COPY_BLOCKSIZE;
    if (len && len < blocksize) blocksize = len;
    ByteBuffer buf;

    while (true)
    {
        auto r = src.readChunk(buf, blocksize);
        if (r.res == OR_Failure) return r;

        if (r.res == OR_EOF) break;

        condret(dst.write(buf));

        if (!len) continue;

        len -= blocksize;
        if (len < blocksize) blocksize = len;

        if (len == 0) break;
    }

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

#if defined(WINDOWS_SYSTEM)
    return File::stat(m_path.toStr());

#else
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
#endif
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
    if (fseeko(m_file, static_cast<int64_t>(pos), SEEK_SET) == -1)
        return {OR_Failure, CerberusUtils::strPrint("fseek fail: %s", strerror(errno))};

    return OR_OK;
}
//=============================================================================
OpRes File::_read(ByteBuffer& buf, LSIZE start, LSIZE span) const
{
    if (!isOpen()) return OR_BadConditions;

    auto sz = size();
    condret_str(sz, "size() error");

    if (start >= sz.value) return {OR_WrongArgument, "start greater than file size"};

    // cap to EOF if span is too large or if span is zero
    if (!span || ((start + span) > sz.value)) span = sz.value - start;

    condret(seek(start));

    return _read_cursor(buf, span);
}
//=============================================================================
OpRes File::_read_cursor(ByteBuffer& buf, LSIZE span) const
{
    if (!isOpen()) return OR_BadConditions;
    if (feof(m_file)) return OR_EOF;

    buf.resize(span);  // create room for at most span bytes

    clearerr(m_file);
    size_t ret = fread(buf.data(), 1, static_cast<size_t>(span), m_file);

    buf.resize(static_cast<LSIZE>(ret));  // re-adjust to actual read bytes count

    if (ferror(m_file)) return OR_Failure;

    if (feof(m_file))
    {
        if (ret)
            return OpRes(OR_OK).addOptional(OR_EOF);
        else
            return OR_EOF;
    }

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
#ifdef WINDOWS_SYSTEM
        ::_close(m_fd);  // RAII
#else
        ::close(m_fd);  // RAII
#endif
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
OpRes File::erase(LSIZE start, LSIZE span)
{
    if (!isOpen()) return OR_BadConditions;

    auto sz = size();
    condret_str(sz, "size() in erase");
    if (start > sz.value) return OR_WrongArgument;
    if (start + span > sz.value) span = sz.value - start;  // clamp to EOF

    // create temp file in same directory when possible
    std::string dirStr = m_path.toStr();
#ifdef WINDOWS_SYSTEM
    auto slash = dirStr.find_last_of("/\\");
#else
    auto slash = dirStr.find_last_of('/');
#endif
    Path tempDir;
    if (slash != std::string::npos && slash > 0)
        tempDir = Path(dirStr.substr(0, slash));
    File tmp = File::tmpFile(tempDir, FOM_ReadWriteTrunc);

    // copy head
    if (start > 0)
    {
        condret(seek(0));
        condret(File::zeroCopy(*this, tmp, start));
    }

    // copy tail
    LSIZE tailStart = start + span;
    if (tailStart < sz.value)
    {
        condret(seek(tailStart));
        condret(File::zeroCopy(*this, tmp, sz.value - tailStart));
    }

    tmp.close();
    close();

    auto mv = File::move(tmp.path().toStr(), m_path.toStr());
    condret_str(mv, "move temp in erase");

    // reopen with previous open mode
    return open();
}
//=============================================================================
OpRes File::move(const Path& newPath)
{
#ifdef WINDOWS_SYSTEM
    const bool wasOpen = isOpen();
    if (wasOpen) close();

    auto res = move(m_path.toStr(), newPath.toStr());

    if (res.ok()) path(newPath);

    if (wasOpen)
    {
        auto reopenRes = open();
        if (res.fail()) return res;
        return reopenRes;
    }

    return res;
#else
    auto res = move(m_path.toStr(), newPath.toStr());

    if (res.ok()) path(newPath);

    return res;
#endif
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

    auto backup = getCursor();
    condret_str(backup, "getCursor() error 1 in size()");
    condret(seekToEOF());
    auto size = getCursor();
    condret_str(size, "getCursor() error 2 in size()");
    condret(_seek(backup.value));
    return (LSIZE)size.value;
}
//=============================================================================
OpRes File::write(const ByteBuffer& bytes)
{
    if (!isOpen()) return OR_BadConditions;

    clearerr(m_file);
    const size_t expected = (size_t)bytes.size();
    const size_t written = fwrite(bytes.data(), 1, expected, m_file);
    if (written != expected)
    {
        const int err = errno;
        return {OR_SystemFailure,
                CerberusUtils::strPrint("fwrite failed (%zu/%zu): %s",
                                       written, expected, strerror(err))};
    }

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
#ifdef WINDOWS_SYSTEM
    std::string tmpPath = tmp.completePath().toStr();
    std::string dstPath = completePath().toStr();
    tmp.close();
    close();
    condret(File::move(tmpPath, dstPath));

    condret(open());
#else
    condret(File::move(tmp.completePath().toStr(), completePath().toStr()));

    condret(reopen());
#endif

    return OR_OK;
}
//=============================================================================
OpRes File::read(ByteBuffer& buf, LSIZE start) const { return _read(buf, start); }
//=============================================================================
OpRes File::read(ByteBuffer& buf, LSIZE start, LSIZE span) const { return _read(buf, start, span); }
//=============================================================================
OpRes File::readChunk(ByteBuffer& buf, LSIZE chunksize) const { return _read_cursor(buf, chunksize); }
//=============================================================================
OpResData<ByteBuffer> File::readUntil(const ByteBuffer& sequence) const
{
    auto backup = getCursor();
    condret(backup);

    auto seqpos = search(sequence);
    condret(seek(backup.value));

    condret(seqpos);

    // sequence found

    ByteBuffer buf;
    auto ret = readChunk(buf, seqpos.value - backup.value);
    condret(ret);

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
    LSIZE seqindex = 0;
    LSIZE seqlast  = sequence.size() - 1;
    LSIZE seqsize  = sequence.size();

    while (true)
    {
        fread(&c, 1, 1, m_file);

        if (ferror(m_file)) return OR_Failure;
        if (feof(m_file)) return OR_NotFound;

        if (c == sequence[seqindex])
        {
            if (seqindex == seqlast) return getCursor().expect().value - seqsize;
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
    if (feof(m_file)) return OR_EOF;

    clearerr(m_file);

    char temp[128];

    std::string line;

    while (true)
    {
        // read end (EOF, error or newline)
        if (fgets(temp, sizeof(temp), m_file) == nullptr) break;

        line += temp;

        if (line.ends_with('\n'))  // full line
        {
            line.pop_back();
            if (line.ends_with('\r')) line.pop_back();
            break;
        }

        if (getCursor().value == size().value) break;
    }

    if (ferror(m_file)) return {OR_Failure, strerror(errno)};

    if (line.empty() && feof(m_file)) return OR_EOF;

    if (line.ends_with('\r')) line.pop_back();

    if (getCursor().value == size().value) return StringOpRes(line).addOptional(OR_EOF);  // helper

    return line;
}
//=============================================================================
OpRes File::seek(crb::LSIZE pos) const
{
    if (!isOpen()) return OR_BadConditions;

    auto sz = size();
    condret_str(sz, "seek() error");

    if (pos >= sz.value) return seekToEOF();

    return _seek(pos);
}
//=============================================================================
OpRes File::seekOffset(OFFSET pos) const
{
    if (!isOpen()) return OR_BadConditions;

    clearerr(m_file);

    if (fseeko(m_file, static_cast<int64_t>(pos), SEEK_CUR) == -1) return {OR_Failure, strerror(errno)};

    return OR_OK;
}
//=============================================================================
OpRes File::seekToEOF() const
{
    if (!isOpen()) return OR_BadConditions;

    clearerr(m_file);

    if (fseeko(m_file, static_cast<int64_t>(0), SEEK_END) == -1) return {OR_Failure, strerror(errno)};

    return OR_OK;
}
//=============================================================================
void File::resetCursor() const { ::rewind(m_file); }
//=============================================================================
SizeOpRes File::getCursor() const
{
    if (!isOpen()) return OR_BadConditions;

    auto pos = ftello(m_file);
    if (pos == static_cast<decltype(pos)>(-1)) return {OR_Failure, strerror(errno)};
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
