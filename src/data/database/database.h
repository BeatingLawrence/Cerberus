#ifndef DATABASE_H
#define DATABASE_H

#include "../../types.h"
#include "idatabase.h"

namespace cerberus
{
    class Database
    {
       private:
        bool m_ready;
        db::IDatabase* m_db;

       public:
        Database(DBBackend backend = DBB_Filesystem);

        virtual ~Database();

        bool isReady() const;

        OpRes init(const std::string& parameters);

        void deinit();

        OpRes command(const std::string& query);

        OpResData<DBTableBlock> queryBlock(const std::string& query);

        OpResData<DBTableProto> queryPrototype(const std::string& tableName);
    };

}  // namespace cerberus

#endif  // DATABASE_H
