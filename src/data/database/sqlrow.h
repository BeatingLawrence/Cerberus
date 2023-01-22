#ifndef CERBERUS_DATA_DATABASE_SQLROW_H
#define CERBERUS_DATA_DATABASE_SQLROW_H

#include <string>
#include <vector>

namespace cerberus
{
    namespace data
    {
        namespace database
        {
            class SQLBlock;
            class SQLDatabase;

            class SQLRow
            {
                    friend class cerberus::data::database::SQLBlock;
                    friend class cerberus::data::database::SQLDatabase;

                private:
                    bool m_failed;

                    std::string m_failureReason;

                    std::vector<std::string> m_values;

                public:
                    SQLRow() = default;

                    SQLRow(const SQLRow& other) = default;

                    ~SQLRow();

                    bool isFailed() const;

                    std::string failureReason() const;

                    SQLRow& operator= (const SQLRow& other);

                    void append(const std::string& value);

                    size_t size() const;

                    void clear();

                    std::string operator [](size_t pos) const;
            };
        }
    }
}

#endif // CERBERUS_DATA_DATABASE_SQLROW_H
