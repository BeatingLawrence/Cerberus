#ifndef CERBERUS_DIRECTORY_H
#define CERBERUS_DIRECTORY_H

#include <list>

#include "../../types.h"
#include "file.h"

namespace cerberus
{
    class Directory
    {
        std::string m_name;

        std::list<File> m_files;
        std::list<Directory> m_dirs;
        Path m_parent;
        LSIZE m_size;

        OpRes _get(int parentFd, bool recursive);

        Directory(Path parents, const std::string& dirName);

       public:
        Directory(const std::string& dirName = "");

        void name(const std::string& dirName);

        std::string name() const;

        std::string path() const;

        std::string completePath() const;

        OpRes get(bool recursive = false);

        std::list<File> files() const;

        std::list<Directory> dirs() const;

        LSIZE size() const;

        void toStr(std::string& str, int level = 0) const;
    };
}  // namespace cerberus

#endif  // CERBERUS_DIRECTORY_H
