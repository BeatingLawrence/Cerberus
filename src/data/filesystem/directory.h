#ifndef CERBERUS_DIRECTORY_H
#define CERBERUS_DIRECTORY_H

#include <list>

#include "../../types.h"
#include "file.h"

namespace crb
{
    class Directory
    {
        Path m_path;
        std::list<File> m_files;
        std::list<Directory> m_dirs;
        LSIZE m_size;

        OpRes _get(int parentFd, bool recursive);

       public:
        Directory(const Path& path);

        void path(const Path& path);

        Path path() const;

        Path completePath() const;

        std::string name() const;

        OpRes get(bool recursive = false);

        std::list<File> files() const;

        std::list<Directory> dirs() const;

        LSIZE size() const;

        void toStr(std::string& str, int level = 0) const;
    };
}  // namespace crb

#endif  // CERBERUS_DIRECTORY_H
