#ifdef POSTGRESQL_SUPPORT

#ifndef POSTGRESDB_H
#define POSTGRESDB_H

#include "idatabase.h"

namespace pqxx
{
    class connection;
}

namespace cerberus
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

            virtual OpRes command(const string& query);

            virtual OpResData<DBTableBlock> queryBlock(const string& query);

            virtual OpResData<DBTableProto> queryPrototype(const string& tableName);

            virtual ~PostgresDB();
        };
    }  // namespace db
}  // namespace cerberus

#endif  // POSTGRESDB_H

#endif  // POSTGRESQL_SUPPORT
