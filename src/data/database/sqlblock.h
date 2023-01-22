#ifndef CERBERUS_DATA_DATABASE_SQLBLOCK_H
#define CERBERUS_DATA_DATABASE_SQLBLOCK_H

/*  This data class represents a group of rows, aka "block" of rows.
 *
 *  It is used to obtain a result from a query or to insert data to a table
 *
 */

#include <cstddef>
#include <string>
#include <vector>

namespace cerberus
{
    namespace data
    {
        namespace database
        {
            class SQLRow;

            class SQLDatabase;

            class SQLBlock
            {
                    friend class cerberus::data::database::SQLDatabase;

                private:
                    bool m_failed;

                    std::string m_failureReason;

                    std::vector<SQLRow> m_rows;

                    size_t m_columns;

                public:
                    SQLBlock() = default;

                    SQLBlock(const SQLBlock& other) = default;

                    SQLBlock(const SQLRow& row);

                    ~SQLBlock();

                    bool isFailed() const;

                    std::string failureReason() const;

                    SQLBlock& operator= (const SQLBlock& other);

                    void append(const SQLRow& row);

                    size_t size() const;

                    bool empty() const;

                    void clear();

                    SQLRow operator[](size_t pos) const;

                    bool operator== (const SQLBlock& other)const;

                    bool operator!= (const SQLBlock& other)const;
            };
        }
    }
}

#endif // CERBERUS_DATA_DATABASE_SQLBLOCK_H
