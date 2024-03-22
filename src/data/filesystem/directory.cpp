#include "directory.h"

#include <dirent.h>
#include <sys/fcntl.h>
#include <sys/types.h>

#include "../../cerberus.h"

using namespace cerberus;

//=============================================================================
void Directory::path(const Path& path) { m_path = path; }
//=============================================================================
Path Directory::path() const { return m_path; }
//=============================================================================
OpRes Directory::_get(int parentFd, bool recursive)
{
    m_files.clear();
    m_dirs.clear();
    m_size = 0;

    if (m_path.empty()) return {OR_Empty, "path is empty"};

    int fd = openat(parentFd, m_path.back().c_str(), O_RDONLY | O_DIRECTORY);

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
OpRes Directory::get(bool recursive) { return _get(AT_FDCWD, recursive); }
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
