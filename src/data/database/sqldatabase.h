#ifndef CERBERUS_SQLDATABASE_H
#define CERBERUS_SQLDATABASE_H

#include <string>

#include "../../types.h"
#include "database.h"

namespace cerberus
{
    class DBTableBlock;

    class DBTableProto;

    class SQLDatabase : public Database
    {
       public:
        SQLDatabase(DBBackend backend = DBB_PostgreSQL);

        virtual ~SQLDatabase();

        OpRes createTable(DBTableProto& prototype);

        /*  This method inserts a block of rows in the table specified by prototype.
         *  Please note that all the information present in the prototype of the block must be
         * correct, even the data types. This method will not do any error-check of such parameters
         *  The result can be:
         *      > OR_OK if the insertion is successfully executed
         *      > OR_QueryFailure if the query has a problem during execution.
         *      > OR_DBFailure if the database encounters a problem and the command failed.
         *        The failure information are obtainable through failureReason()
         */
        OpRes insertBlock(const DBTableBlock& block);

        /*  This method drops a table.
         *  The result can be:
         *      > OR_OK if the drop is successfully executed
         *      > OR_QueryFailure if the query has a problem during execution.
         *      > OR_DBFailure if the database encounters a problem and the command failed.
         *        The failure information are obtainable through failureReason()
         */
        OpRes dropTable(const std::string& table);

        /*  This method performs a query and returns an entire table.
         *  Be careful with this method, because if the requested table is huge,
         *  an insane amount of memory will be allocated
         *
         * The result can be:
         *      > OR_OK if the query is successfully completed.
         *      > OR_QueryFailure if the query had a problem during execution.
         *      > OR_DBFailure if the database encounters a problem and the query failed.
         *      > OR_NotFound if the query was successfully completed but gave no information
         *
         *  The given block is a structured block
         */
        OpResData<DBTableBlock> querytable(const std::string& tableName);
    };
}  // namespace cerberus

#endif  // CERBERUS_SQLDATABASE_H
