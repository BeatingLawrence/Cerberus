#ifndef CERBERUS_DATA_DATABASE_SQLRESULT_H
#define CERBERUS_DATA_DATABASE_SQLRESULT_H

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

            class SQLResult
            {
                    friend class cerberus::data::database::SQLDatabase;

                private:
                    bool m_failed;

                    std::string m_failureReason;

                    std::vector<SQLRow> m_rows;

                    size_t m_columns;

                public:
                    SQLResult() = default;

                    SQLResult(const SQLResult& other) = default;

                    ~SQLResult();

                    bool isFailed() const;

                    std::string failureReason() const;

                    SQLResult& operator= (const SQLResult& other);

                    void append(const SQLRow& row);

                    size_t size() const;

                    SQLRow operator[](size_t pos) const;

                    bool operator== (const SQLResult& other)const;

                    bool operator!= (const SQLResult& other)const;
            };
        }
    }
}

#endif // CERBERUS_DATA_DATABASE_SQLRESULT_H
