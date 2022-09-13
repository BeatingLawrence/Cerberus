#ifndef CERBERUS_DATA_DATABASE_SQLROW_H
#define CERBERUS_DATA_DATABASE_SQLROW_H

#include <cstddef>
#include <string>
#include <tuple>

namespace pqxx
{
    class row;
}

namespace cerberus
{
    namespace data
    {
        namespace database
        {
            class SQLResult;
            class SQLDatabase;

            class SQLRow
            {
                    friend cerberus::data::database::SQLResult;
                    friend cerberus::data::database::SQLDatabase;

                private:
                    pqxx::row* m_row;

                    bool m_failed;

                    std::string m_failureReason;

                public:
                    SQLRow();

                    SQLRow(const SQLRow& other);

                    SQLRow(SQLRow&& other);

                    ~SQLRow();

                    bool isFailed() const;

                    std::string failureReason() const;

                    SQLRow& operator=(const SQLRow& other);

                    size_t size()const;

                    template<typename... TYPE>
                    std::tuple<TYPE...> to() const;
            };
        }
    }
}

#endif // CERBERUS_DATA_DATABASE_SQLROW_H
