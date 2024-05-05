#ifndef IDATABASE_H
#define IDATABASE_H

/*  This file describes the database interface.
 *
 *  New database backends must implement this interface to
 *  be used as database interface
 *
 */

#include "../../types.h"
#include "dbdata.h"

using namespace std;

namespace cerberus
{
    namespace db
    {
        class IDatabase
        {
           protected:
            bool m_ready;

            IDatabase()
                : m_ready(false){};

           public:
            virtual OpRes init(const std::string& parameters) = 0;

            virtual void deinit() = 0;

            inline bool ready() const { return m_ready; };

            virtual OpRes command(const string& command) = 0;

            virtual OpResData<DBTableBlock> queryBlock(const string& query) = 0;

            virtual OpResData<DBTableProto> queryPrototype(const string& tableName) = 0;

            virtual ~IDatabase(){};
        };
    }  // namespace db
}  // namespace cerberus

#endif  // IDATABASE_H
