#ifndef FILESYSTEMDB_H
#define FILESYSTEMDB_H

#include "idatabase.h"

namespace cerberus
{
    namespace db
    {
        class FilesystemDB : public IDatabase
        {
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
