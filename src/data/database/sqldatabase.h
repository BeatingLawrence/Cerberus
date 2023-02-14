#ifndef CERBERUS_DATA_DATABASE_SQLDATABASE_H
#define CERBERUS_DATA_DATABASE_SQLDATABASE_H

/*  This class provides an interface for the PostgreSQL pqxx C++ library
 *
 *  To connect to a database, simply invoke the constructor passing the parameter string.
 *  For more information about parameter string composing, please visit:
 *  https://www.postgresql.org/docs/current/libpq-connect.html#LIBPQ-CONNSTRING
 *
 *  Once the constructor terminates, please check the failure state of the object using isFailed().
 *  Other information about failure can be retrieved with failureReason().
 *
 *
 *  It is possible to communicate with the database through query() querySingleRow() and command() calls.
 *
 *  Theese three querier are divided into two main categories:
 *      - non-retrieving:   those which are not meant for retrieving data from the database,
 *                          but just for inserting/deleting/modifying.
 *                          command() falls into this category.
 *
 *      - retrieving:       those which are made for getting data from the database.
 *                          query() and querySingleRow() fall into this category.
 *
 *  It is important to distinguish the two, because (e.g.) using command() with
 *  a retrieving query as parameter, will cause command() call to fail and return false.
 *
 */

#include <string>

namespace pqxx
{
    class connection;
}

namespace cerberus
{
    namespace data
    {
        namespace database
        {
            class SQLBlock;

            class SQLTablePrototype;

            class SQLDatabase
            {
                private:
                    pqxx::connection* m_connection;

                    bool m_failed;

                    std::string m_failureReason;

                public:
                    enum OperationResult
                    {
                        OR_OK,
                        OR_QUERY_FAILURE,
                        OR_DB_FAILURE,
                        OR_NOT_FOUND,
                        OR_TABLE_ALREADY_PRESENT,
                        // add more here
                    };

                    SQLDatabase() = delete;

                    ~SQLDatabase();

                    bool isFailed() const;

                    std::string failureReason() const;

                    SQLDatabase(const std::string& parameters) noexcept;

                    /*  This method performs a query and returns a result.
                     *  The result can be:
                     *      > OR_OK if the query is successfully completed. The queried information are inside output parameter
                     *      > OR_QUERY_FAIL if the query has a problem during execution. The failure information are inside output parameter
                     *      > OR_DB_FAIL if the database encounters a problem and the query failed.
                     *        The failure information are obtainable through failureReason()
                     *      > OR_NOT_FOUND if the query was successfully completed but gave no information (0 results found)
                     *
                     *  The given block is an unstructured block
                     */
                    OperationResult queryBlock(const std::string& query, SQLBlock& output);

                    /*  This method performs a query and returns a result.
                     *  This method also alters the given prototype to match the correct one of the queried table
                     *  The result can be:
                     *      > OR_OK if the query is successfully completed. The queried information are inside prototype parameter
                     *      > OR_QUERY_FAIL if the query has a problem during execution. The failure information are inside prototype parameter
                     *      > OR_DB_FAIL if the database encounters a problem and the query failed.
                     *        The failure information are obtainable through failureReason()
                     *      > OR_NOT_FOUND if the query was successfully completed but no such table exists
                     */
                    OperationResult queryPrototype(SQLTablePrototype& prototype);

                    /*  This method executes a command and returns a result.
                     *  The result can be:
                     *      > OR_OK if the command is successfully executed
                     *      > OR_QUERY_FAIL if the query has a problem during execution.
                     *      > OR_DB_FAIL if the database encounters a problem and the command failed.
                     *        The failure information are obtainable through failureReason()
                     */
                    OperationResult command(const std::string& query);

                    /*  This method creates a table in the database.
                     *  The result can be:
                     *      > OR_OK if the creation is successfully executed and the table has been created
                     *      > OR_QUERY_FAIL if the query has a problem during execution.
                     *      > OR_DB_FAIL if the database encounters a problem and the command failed.
                     *        The failure information are obtainable through failureReason()
                     *      > OR_TABLE_ALREADY_PRESENT if the specified table already exists and could not be created
                     */
                    OperationResult createTable(SQLTablePrototype& prototype);

                    /*  This method inserts a block of rows in the table specified by prototype.
                     *  Please note that all the information present in the prototype of the block must be correct, even the data types.
                     *  This method will not do any error-check of such parameters
                     *  The result can be:
                     *      > OR_OK if the insertion is successfully executed
                     *      > OR_QUERY_FAIL if the query has a problem during execution.
                     *      > OR_DB_FAIL if the database encounters a problem and the command failed.
                     *        The failure information are obtainable through failureReason()
                     */
                    OperationResult insertBlock(const SQLBlock& block);

                    /*  This method drops a table.
                     *  The result can be:
                     *      > OR_OK if the drop is successfully executed
                     *      > OR_QUERY_FAIL if the query has a problem during execution.
                     *      > OR_DB_FAIL if the database encounters a problem and the command failed.
                     *        The failure information are obtainable through failureReason()
                     */
                    OperationResult dropTable(const std::string& table);

                    /*  This method performs a query and returns an entire table.
                     *  Be careful with this method, because if the requested table is huge, an insane amount of memory will be allocated
                     *  The result can be:
                     *      > OR_OK if the query is successfully completed. The queried information are inside output parameter
                     *      > OR_QUERY_FAIL if the query has a problem during execution. The failure information are inside output parameter
                     *      > OR_DB_FAIL if the database encounters a problem and the query failed.
                     *        The failure information are obtainable through failureReason()
                     *      > OR_NOT_FOUND if the query was successfully completed but gave no information (0 results found)
                     *
                     *  The given block is a structured block
                     */
                    OperationResult querytable(const std::string& tableName, SQLBlock& output);
            };
        }
    }
}

#endif // CERBERUS_DATA_DATABASE_SQLDATABASE_H
