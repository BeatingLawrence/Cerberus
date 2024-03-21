#include "directory.h"

#include <dirent.h>
#include <sys/fcntl.h>
#include <sys/types.h>

#include "../../cerberus.h"

using namespace cerberus;

//=============================================================================
Directory::Directory(const std::string& dirName)
    : m_name(dirName)
{
}
//=============================================================================
OpRes Directory::_get(int parentFd, bool recursive)
{
    m_files.clear();
    m_dirs.clear();
    m_parent.clear();
    m_size = 0;

    int fd = openat(parentFd, m_name.c_str(), O_RDONLY | O_DIRECTORY);

    if (fd == -1) return {OR_Failure, strerror(errno)};

    auto dir = fdopendir(fd);

    if (!dir) return {OR_Failure, strerror(errno)};

    while (true)
    {
        errno   = 0;
        auto el = readdir(dir);

        if (el == nullptr)
        {
            if (errno == 0) break;  // end of directory

            logError("error in readdir %s", strerror(errno));
            break;
        }

        if ((strcmp(el->d_name, ".") == 0) || (strcmp(el->d_name, "..") == 0)) continue;  // skip

        if (el->d_type == DT_DIR)
            m_dirs.push_back(Directory(m_parent.copy_append(m_name), el->d_name));

        else if (el->d_type == DT_REG)
        {
            m_files.push_back(File(m_parent.copy_append(m_name), el->d_name));
            m_size += m_files.back().size().expect().value;
        }
    }

    if (recursive)
        for (auto& el : m_dirs)
        {
            el._get(fd, true).expect();
            m_size += el.size();
        }

    closedir(dir);

    return OR_OK;
}
//=============================================================================
Directory::Directory(Path parents, const std::string& dirName)
    : m_name(dirName),
      m_parent(parents)
{
}
//=============================================================================
void Directory::name(const std::string& dirName) { m_name = dirName; }
//=============================================================================
std::string Directory::name() const { return m_name; }
//=============================================================================
std::string Directory::path() const { return m_parent.copy_append(m_name).toStr(); }
//=============================================================================
std::string Directory::completePath() const { return CerberusUtils::completePath(path()).value; }
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
