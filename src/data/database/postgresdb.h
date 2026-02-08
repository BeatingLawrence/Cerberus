#ifdef POSTGRESQL_SUPPORT

#ifndef POSTGRESDB_H
#define POSTGRESDB_H

#include "idatabase.h"

namespace pqxx
{
    class connection;
}

namespace crb
{
    namespace db
    {
        class PostgresDB : public IDatabase
        {
            pqxx::connection* m_connection;

            void _deinit();

           public:
            PostgresDB();

            virtual OpRes init(const std::string& parameters);

            virtual void deinit();

            virtual bool ready() const;

            virtual OpRes command(const string& query);

            virtual OpResData<DBTableBlock> queryBlock(const string& query);
            virtual OpResData<DBTableBlock> queryBlock(const DBQuery& query);

            virtual OpResData<DBTableProto> queryPrototype(const string& tableName);

            virtual OpRes createTable(const DBTableProto& prototype);

            virtual OpRes insertBlock(const DBTableBlock& block);

            virtual OpRes updateBlock(const DBTableBlock& block, UpdatePolicy policy = UP_UpdateInsert);

            virtual OpRes dropTable(const std::string& table);
            virtual OpRes renameColumn(const std::string& table, const std::string& oldName,
                                       const std::string& newName);

            virtual OpResData<DBTableBlock> querytable(const std::string& tableName);

            virtual ~PostgresDB();
        };
    }  // namespace db
}  // namespace crb

#endif  // POSTGRESDB_H

#endif  // POSTGRESQL_SUPPORT
