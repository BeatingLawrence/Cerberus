#ifndef DATABASE_H
#define DATABASE_H

#include "../../types.h"
#include "idatabase.h"

namespace crb
{
    class Database
    {
       private:
        db::IDatabase* m_db;

       public:
        Database(DBBackend backend = DBB_Filesystem);

        virtual ~Database();

        inline OpRes init(const std::string& parameters) { return m_db->init(parameters); }

        inline void deinit() { m_db->deinit(); }

        inline bool ready() { return m_db->ready(); }

        inline OpRes command(const std::string& query) { return m_db->command(query); }

        inline OpResData<DBTableBlock> queryBlock(const std::string& query)
        {
            return m_db->queryBlock(query);
        }
        inline OpResData<DBTableBlock> queryBlock(const DBQuery& query) { return m_db->queryBlock(query); }

        inline OpResData<DBTableProto> queryPrototype(const std::string& tableName)
        {
            return m_db->queryPrototype(tableName);
        }

        inline OpRes createTable(const DBTableProto& prototype) { return m_db->createTable(prototype); }

        inline OpRes insertBlock(const DBTableBlock& block) { return m_db->insertBlock(block); }

        inline OpRes updateBlock(const DBTableBlock& block, UpdatePolicy policy = UP_UpdateInsert)
        {
            return m_db->updateBlock(block, policy);
        }

        inline OpRes dropTable(const std::string& table) { return m_db->dropTable(table); }

        inline OpRes renameColumn(const std::string& table, const std::string& oldName,
                                  const std::string& newName)
        {
            return m_db->renameColumn(table, oldName, newName);
        }

        inline OpResData<DBTableBlock> querytable(const std::string& tableName)
        {
            return m_db->querytable(tableName);
        }
    };

}  // namespace crb

#endif  // DATABASE_H
