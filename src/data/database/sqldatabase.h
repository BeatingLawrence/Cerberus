#ifndef CERBERUS_DATA_DATABASE_SQLDATABASE_H
#define CERBERUS_DATA_DATABASE_SQLDATABASE_H

/*  This class provides a wrapper for the PostgreSQL pqxx C++ library
 *
 *  To connect to a database, simply invoke the constructor passing the parameters string.
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
            class SQLResult;

            class SQLRow;

            class SQLDatabase
            {
                private:
                    pqxx::connection* m_connection;

                    bool m_failed;

                    std::string m_failureReason;

                public:
                    SQLDatabase() = delete;

                    ~SQLDatabase();

                    bool isFailed() const;

                    std::string failureReason() const;

                    SQLDatabase(const std::string& parameters) noexcept;

                    //This method performs a query, returning a result.
                    //If a query error occurs, the resulting result will be failed
                    //If another error occurs, the resulting result will be failed, as the database instance will be
                    SQLResult query(const std::string& query);

                    //This method performs a query, returning a single row.
                    //If a query error occurs, the resulting result will be failed
                    //If another error occurs, the resulting result will be failed, as the database instance will be
                    SQLRow querySingleRow(const std::string& query);

                    //This method performs a query which must not return values.
                    //This method will return true if query succeeded, and false in any other cases.
                    //Check database failure flag and reason to tell query and connection errors apart.
                    bool command(const std::string& query);
            };
        }
    }
}

#endif // CERBERUS_DATA_DATABASE_SQLDATABASE_H
