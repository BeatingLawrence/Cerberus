#ifndef CERBERUS_DBDATA_H
#define CERBERUS_DBDATA_H

/*  The DBTableBlock data class represents a group of rows, aka "block" of rows.
 *  It is used to obtain a result from a query or to insert data into a table.
 *  The DBTableBlock does not necessarily have to be structured. The structure property of a block
 *  provides types-convertion functionalities whereas non-structured blocks can contain data anyway
 *
 *  The DBRow data class represents a single row of a table
 *
 *  The DBTableProto data class represents a structure of a table and contains no data
 *
 */

#include <cstddef>
#include <string>
#include <vector>

#include "../../core/cerberusutils.h"
#include "../../types.h"

namespace cerberus
{
    class SQLDatabase;

    class DBCell
    {
       private:
        std::string m_value;

       public:
        DBCell() = default;

        DBCell(double value);
        DBCell(float value);
        DBCell(bool value);
        DBCell(const std::vector<bool>& value);
        DBCell(int64_t value);
        DBCell(int value);
        DBCell(unsigned int value);
        DBCell(const std::string& raw);
        DBCell(const char* value);

        void set(const std::string& value);
        void set(int64_t value);
        void set(double value);
        void set(bool value);
        void set(const std::vector<bool>& value);

        std::string raw() const;

        int64_t toInt() const;

        double toFloat() const;

        bool toBool() const;

        std::vector<bool> toBits() const;

        bool isEqual(const DBCell& other) const;

        bool isEqual(const std::string& str) const;
    };

    class DBRow
    {
       private:
        std::vector<DBCell> m_values;

       public:
        DBRow() = default;

        DBRow(const DBRow& other) = default;

        DBRow& operator=(const DBRow& other) = default;

        ~DBRow();

        void append(const DBCell& value);

        size_t size() const;

        void clear();

        const DBCell& operator[](size_t pos) const;
        DBCell& operator[](size_t pos);

        Iterator<DBCell> begin();
        Iterator<DBCell> end();

        ConstIterator<DBCell> begin() const;
        ConstIterator<DBCell> end() const;
    };

    struct SQLColumn
    {
       public:
        SQLColumn() = delete;
        SQLColumn(const std::string& name, SQLDataType type, int mod = -1)
            : m_columnName(name),
              m_type(type),
              m_mod(mod){};

        std::string name() const { return m_columnName; };

        SQLDataType type() const { return m_type; };

        int mod() const { return m_mod; };

        std::string typeString() const { return CerberusUtils::fromSQLDataType(m_type); };

       private:
        std::string m_columnName;
        SQLDataType m_type;
        int m_mod;
    };

    class DBTableProto
    {
       private:
        std::string m_name;  // the table name

        std::vector<SQLColumn> m_types;

       public:
        DBTableProto(const std::string& name = "");

        DBTableProto& add(const std::string& name, SQLDataType type, int mod = -1);

        const SQLColumn& operator[](int index) const;
        SQLColumn& operator[](int index);

        void clear();

        size_t size() const;

        std::string name() const;

        Iterator<SQLColumn> begin();
        Iterator<SQLColumn> end();

        ConstIterator<SQLColumn> begin() const;
        ConstIterator<SQLColumn> end() const;
    };

    class DBTableBlock
    {
        friend class cerberus::SQLDatabase;

       private:
        std::vector<DBRow> m_rows;

        DBTableProto m_prototype;

       public:
        DBTableBlock();

        DBTableBlock(const std::string& name);

        DBTableBlock(const DBTableProto& proto);

        DBTableBlock(const DBTableBlock& other) = default;

        ~DBTableBlock();

        bool append(const DBRow& row);

        size_t size() const;

        bool empty() const;

        void clear();

        bool structured() const;

        void clearStructure();

        void clearRows();

        void setPrototype(const DBTableProto& prototype);

        DBTableProto prototype() const;

        DBTableBlock& addColumn(const std::string& name, SQLDataType type, int mod = -1);

        const DBRow& operator[](size_t pos) const;
        DBRow& operator[](size_t pos);

        bool operator==(const DBTableBlock& other) const;
        bool operator!=(const DBTableBlock& other) const;

        Iterator<DBRow> begin();
        Iterator<DBRow> end();

        ConstIterator<DBRow> begin() const;
        ConstIterator<DBRow> end() const;
    };
}  // namespace cerberus

#endif  // CERBERUS_DBDATA_H
