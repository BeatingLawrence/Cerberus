#ifndef CERBERUS_DATA_DATABASE_SQLRESULT_H
#define CERBERUS_DATA_DATABASE_SQLRESULT_H

#include "sqlrow.h"
#include <cstddef>
#include <string>

namespace pqxx
{
    class result;
}

namespace cerberus
{
    namespace data
    {
        namespace database
        {
            class SQLDatabase;

            class SQLResult
            {
                    friend cerberus::data::database::SQLDatabase;

                private:
                    pqxx::result* m_result;

                    bool m_failed;

                    std::string m_failureReason;

                public:
                    SQLResult();

                    SQLResult(const SQLResult& other);

                    ~SQLResult();

                    SQLResult& operator= (const SQLResult& other);

                    bool operator== (const SQLResult& other)const;

                    bool operator!= (const SQLResult& other)const;

                    SQLRow operator[](size_t pos) const;

                    size_t size() const;

                    bool isFailed() const;

                    std::string failureReason() const;
            };
        }
    }
}

#endif // CERBERUS_DATA_DATABASE_SQLRESULT_H
