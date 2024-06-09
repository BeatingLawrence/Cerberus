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
            struct Table
            {
                DBTableProto proto;
                LSIZE start;   // position in file where the table begins
                LSIZE buffer;  // position in file where the table data begins

                Table()
                    : proto(),
                      start(0),
                      buffer(0){};
            };

            File m_file;

            std::vector<Table> m_tables;

            StringOpRes _readStr();

            DBMOD _getMod(const ByteBuffer& buf);

            OpRes _parseTable();

            OpRes _load();

           public:
            FilesystemDB();

            virtual OpRes init(const std::string& parameters);

            virtual void deinit();

            virtual OpRes command(const string& query);

            virtual OpResData<DBTableBlock> queryBlock(const string& query);

            virtual OpResData<DBTableProto> queryPrototype(const string& tableName);

            virtual OpRes createTable(DBTableProto& prototype);

            virtual OpRes insertBlock(const DBTableBlock& block);

            virtual OpRes dropTable(const std::string& table);

            virtual OpResData<DBTableBlock> querytable(const std::string& tableName);

            virtual ~FilesystemDB();
        };
    }  // namespace db
}  // namespace cerberus

#endif  // FILESYSTEMDB_H
