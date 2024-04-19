#ifndef FILESYSTEMDB_H
#define FILESYSTEMDB_H

#include "../../data/filesystem/file.h"
#include "idatabase.h"

namespace cerberus
{
    namespace db
    {
        class FilesystemDB : public IDatabase
        {
            File m_file;

           public:
            FilesystemDB();

            virtual OpRes init(const std::string& parameters);

            virtual void deinit();

            virtual OpRes command(const string& query);

            virtual OpResData<DBTableBlock> queryBlock(const string& query);

            virtual OpResData<DBTableProto> queryPrototype(const string& tableName);

            virtual ~FilesystemDB();
        };
    }  // namespace db
}  // namespace cerberus

#endif  // FILESYSTEMDB_H
