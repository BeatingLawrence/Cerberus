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
#include "../bytebuffer.h"

namespace cerberus
{
    class DBCell
    {
        ByteBuffer m_value;

        void _fromBitArray(const std::vector<bool>& arr);

       public:
        DBCell() = default;

        DBCell(int64_t value);
        DBCell(int32_t value);
        DBCell(float value);
        DBCell(long double value);
        DBCell(bool value);
        DBCell(const std::vector<bool>& value);
        DBCell(const std::string& str);
        DBCell(const char* str);
        DBCell(const ByteBuffer& raw);

        void set(const std::string& str);
        void set(const char* str);
        void set(int64_t value);
        void set(long double value);
        void set(bool value);
        void set(const std::vector<bool>& value);

        ByteBuffer& raw();
        const ByteBuffer& raw() const;

        ByteBuffer serialize(DBDataType type, DBMOD mod) const;

        int64_t toInt() const;
        long double toReal() const;
        bool toBool() const;
        std::vector<bool> toBits() const;

        LSIZE size() const;

        bool isEqual(const DBCell& other) const;
    };

    class DBTableProto;

    class DBRow
    {
        std::vector<DBCell> m_values;

       public:
        DBRow() = default;

        DBRow(const DBRow& other) = default;

        DBRow& operator=(const DBRow& other) = default;

        ~DBRow();

        void append(const DBCell& value);

        size_t size() const;

        void clear();

        bool verify(const DBTableProto& proto) const;

        ByteBuffer serialize(const DBTableProto& proto) const;

        const DBCell& operator[](size_t pos) const;
        DBCell& operator[](size_t pos);

        Iterator<DBCell> begin();
        Iterator<DBCell> end();

        ConstIterator<DBCell> begin() const;
        ConstIterator<DBCell> end() const;
    };

    struct DBColumn
    {
       public:
        DBColumn() = delete;
        DBColumn(const std::string& name, DBDataType type, DBMOD mod = 0)
            : m_columnName(name),
              m_type(type),
              m_mod(mod) {};

        std::string name() const { return m_columnName; };
        void setName(const std::string& name) { m_columnName = name; };

        DBDataType type() const { return m_type; };

        DBMOD mod() const { return m_mod; };

        std::string typeString() const { return CerberusUtils::fromDBDataType(m_type); };

        bool operator==(const DBColumn& other) const;

       private:
        std::string m_columnName;
        DBDataType m_type;
        DBMOD m_mod;
    };

    class DBTableProto
    {
        std::string m_name;  // the table name

        std::vector<DBColumn> m_types;

       public:
        DBTableProto(const std::string& name = "");

        DBTableProto& add(const std::string& name, DBDataType type, DBMOD mod = 0);

        const DBColumn& operator[](int index) const;
        DBColumn& operator[](int index);

        void clear();

        size_t size() const;

        bool isEqual(const DBTableProto& other) const;

        std::string name() const;
        void setName(const std::string& name);

        Iterator<DBColumn> begin();
        Iterator<DBColumn> end();

        ConstIterator<DBColumn> begin() const;
        ConstIterator<DBColumn> end() const;
    };

    class DBTableBlock
    {
        std::vector<DBRow> m_rows;

        DBTableProto m_prototype;

       public:
        DBTableBlock() = default;

        DBTableBlock(const DBTableBlock& other) = default;

        DBTableBlock(const std::string& name);

        DBTableBlock(const DBTableProto& proto);

        DBTableBlock(const DBRow& row);

        ~DBTableBlock();

        OpRes append(const DBRow& row);

        size_t size() const;

        bool empty() const;

        void clear();

        bool structured() const;

        void clearStructure();

        void clearRows();

        void setPrototype(const DBTableProto& prototype);

        void assignRows(const DBTableBlock& other);

        bool verify() const;

        bool verify(const DBTableProto& proto) const;

        ByteBuffer serialize(const DBTableProto& proto) const;

        const DBTableProto& prototype() const;

        DBTableBlock& addColumn(const std::string& name, DBDataType type, DBMOD mod = 0);

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
