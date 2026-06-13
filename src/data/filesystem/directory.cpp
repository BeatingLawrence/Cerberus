#include "directory.h"

#ifdef WINDOWS_SYSTEM
#include <filesystem>
#else
#include <dirent.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#endif
#include <string.h>

#include "../../cerberus.h"

using namespace crb;

//=============================================================================
void Directory::path(const Path& path) { m_path = path; }
//=============================================================================
Path Directory::path() const { return m_path; }
//=============================================================================
OpRes Directory::_get(int parentFd, bool recursive)
{
#ifdef WINDOWS_SYSTEM
    (void)parentFd;
    m_files.clear();
    m_dirs.clear();
    m_size = 0;

    if (m_path.empty()) return {OR_Empty, "path is empty"};

    std::error_code ec;
    const std::filesystem::path base(m_path.toStr());
    if (!std::filesystem::is_directory(base, ec)) return OR_InvalidPath;

    for (const auto& entry : std::filesystem::directory_iterator(base, ec))
    {
        if (ec) return {OR_Failure, ec.message()};

        const std::string name = entry.path().filename().string();
        Path child             = m_path.copy_append(name);

        if (entry.is_directory(ec))
        {
            m_dirs.push_back(Directory(child));
        }
        else if (entry.is_regular_file(ec))
        {
            File f(child);
            m_files.push_back(f);

            auto res = f.size();
            if (res.ok("error when getting file size")) m_size += res.value;
        }
    }

    if (ec) return {OR_Failure, ec.message()};

    if (recursive)
        for (auto& el : m_dirs)
        {
            auto res = el._get(0, true);
            if (res.fail()) return res;
            m_size += el.size();
        }

    return OR_OK;
#else
    m_files.clear();
    m_dirs.clear();
    m_size = 0;

    if (m_path.empty()) return {OR_Empty, "path is empty"};

    // If we're opening from the filesystem root (AT_FDCWD), we must use the full
    // path string (it can be multi-component or absolute).
    // If we're recursing using a parent directory fd, we must open only the last
    // component relative to that fd.
    const std::string openPath = (parentFd == AT_FDCWD) ? m_path.toStr() : m_path.back();
    int fd = openat(parentFd, openPath.c_str(), O_RDONLY | O_DIRECTORY);

    if (fd == -1) return {OR_Failure, "openat error", strerror(errno)};

    auto dir = fdopendir(fd);

    if (!dir)
    {
        ::close(fd);
        return {OR_Failure, "fdopendir error", strerror(errno)};
    }

    while (true)
    {
        errno   = 0;
        auto el = readdir(dir);

        if (el == nullptr)
        {
            if (errno == 0) break;  // end of directory

            closedir(dir);
            return {OR_Failure, "readdir error", strerror(errno)};
        }

        if ((strcmp(el->d_name, ".") == 0) || (strcmp(el->d_name, "..") == 0)) continue;  // skip

        if (el->d_type == DT_DIR)
            m_dirs.push_back(Directory(m_path.copy_append(el->d_name)));

        else if (el->d_type == DT_REG)
        {
            File f(m_path.copy_append(el->d_name));
            m_files.push_back(f);

            auto res = f.size();
            if (res.ok("error when getting file size")) m_size += res.value;
        }
    }

    if (recursive)
        for (auto& el : m_dirs)
        {
            auto res = el._get(fd, true);

            if (res.fail())
            {
                closedir(dir);
                return res;
            }

            m_size += el.size();
        }

    closedir(dir);

    return OR_OK;
#endif
}
//=============================================================================
Directory::Directory(const Path& path)
    : m_path(path)
{
}
//=============================================================================
Path Directory::completePath() const { return CerberusUtils::completePath(m_path.toStr()).value; }
//=============================================================================
std::string Directory::name() const { return m_path.back(); }
//=============================================================================
OpRes Directory::get(bool recursive)
{
#ifdef WINDOWS_SYSTEM
    return _get(0, recursive);
#else
    return _get(AT_FDCWD, recursive);
#endif
}
//=============================================================================
std::list<File> Directory::files() const { return m_files; }
//=============================================================================
std::list<Directory> Directory::dirs() const { return m_dirs; }
//=============================================================================
LSIZE Directory::size() const { return m_size; }
//=============================================================================
void Directory::toStr(std::string& str, int level) const
{
    std::string indent(level * 2, ' ');

    str.append("\n").append(indent);

    for (auto& el : m_files)
    {
        str.append(el.name()).append("\n").append(indent);
    }

    for (auto& el : m_dirs)
    {
        str.append(el.name()).append(":");
        el.toStr(str, level + 1);
    }

    if (indent.empty()) return;
    str.pop_back();
    str.pop_back();
}
//=============================================================================
