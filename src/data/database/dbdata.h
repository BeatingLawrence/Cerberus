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
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../../core/cerberusutils.h"
#include "../../types.h"
#include "../bytebuffer.h"

namespace crb
{
    class DBCell
    {
        ByteBuffer m_value;

        void _fromBitArray(const std::vector<bool>& arr);

       public:
        DBCell() = default;

        DBCell(int64_t value);
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

    enum DBQueryConditionType : uint8_t
    {
        DBQC_None = 0,
        DBQC_Range,
        DBQC_Exact,
        DBQC_Regex,
    };

    struct DBQueryCondition
    {
        DBQueryConditionType type;
        std::string column;
        std::optional<long double> min;
        std::optional<long double> max;
        std::string exactValue;
        std::string regexPattern;
        bool invertOrder;

        DBQueryCondition()
            : type(DBQC_None),
              column(),
              min(),
              max(),
              exactValue(),
              regexPattern(),
              invertOrder(false)
        {
        }
    };

    class DBQuery
    {
        std::string m_table;
        DBQueryCondition m_condition;
        std::vector<std::string> m_columns;

       public:
        DBQuery() = default;
        explicit DBQuery(const std::string& tableName);

        const std::string& table() const;
        void setTable(const std::string& tableName);

        const DBQueryCondition& condition() const;
        DBQueryCondition& condition();

        const std::vector<std::string>& columns() const;
        void setColumns(const std::vector<std::string>& columns);
        void addColumn(const std::string& column);
        bool selectAllColumns() const;

        OpRes validate() const;

        static OpResData<DBQuery> fromString(const std::string& query);
    };

    class DBRow
    {
        std::vector<DBCell> m_values;
        const DBTableProto* m_protoRef;
        std::unique_ptr<DBTableProto> m_protoOwned;

       public:
        DBRow();

        DBRow(const DBRow& other);

        DBRow& operator=(const DBRow& other);

        DBRow(DBRow&& other) noexcept = default;
        DBRow& operator=(DBRow&& other) noexcept = default;

        ~DBRow();

        void bindPrototype(const DBTableProto* proto);
        void detachPrototype();
        bool hasPrototype() const;
        const DBTableProto* prototype() const;

        void append(const DBCell& value);
        void append(int64_t v) { append(DBCell(v)); }
        void append(int32_t v) { append(DBCell((int64_t)v)); }
        void append(int16_t v) { append(DBCell((int64_t)v)); }
        void append(int8_t v) { append(DBCell((int64_t)v)); }
        void append(uint64_t v) { append(DBCell((int64_t)v)); }
        void append(uint32_t v) { append(DBCell((int64_t)v)); }
        void append(uint16_t v) { append(DBCell((int64_t)v)); }
        void append(uint8_t v) { append(DBCell((int64_t)v)); }
        void append(long double v) { append(DBCell(v)); }
        void append(double v) { append(DBCell((long double)v)); }
        void append(float v) { append(DBCell(v)); }
        void append(bool v) { append(DBCell(v)); }
        void append(const std::string& v) { append(DBCell(v)); }
        void append(const char* v) { append(DBCell(v)); }
        void append(const std::vector<bool>& v) { append(DBCell(v)); }

        size_t size() const;

        void clear();

        bool verify(const DBTableProto& proto) const;

        ByteBuffer serialize(const DBTableProto& proto) const;

        const DBCell& operator[](size_t pos) const;
        DBCell& operator[](size_t pos);
        const DBCell& operator[](const std::string& column) const;
        DBCell& operator[](const std::string& column);

        Iterator<DBCell> begin();
        Iterator<DBCell> end();

        ConstIterator<DBCell> begin() const;
        ConstIterator<DBCell> end() const;
    };

    struct DBColumn
    {
       public:
        DBColumn() = delete;
        DBColumn(const std::string& name, DBDataType type, DBMOD mod = 0, DBFLAGS flags = DBF_None,
                 const DBCell& defaultValue = DBCell())
            : m_columnName(name),
              m_type(type),
              m_mod(mod),
              m_flags(flags),
              m_defaultValue(defaultValue) {};

        std::string name() const { return m_columnName; };
        void setName(const std::string& name) { m_columnName = name; };

        DBDataType type() const { return m_type; };

        DBMOD mod() const { return m_mod; };

        DBFLAGS flags() const { return m_flags; };

        std::string typeString() const { return CerberusUtils::fromDBDataType(m_type); };

        bool isPrimary() const { return (m_flags & DBF_PrimaryKey) != 0; };

        const DBCell& defaultValue() const { return m_defaultValue; };
        void setDefaultValue(const DBCell& value) { m_defaultValue = value; };

        bool operator==(const DBColumn& other) const;

       private:
        std::string m_columnName;
        DBDataType m_type;
        DBMOD m_mod;
        DBFLAGS m_flags;
        DBCell m_defaultValue;
    };

    class DBTableProto
    {
        std::string m_name;  // the table name

        std::vector<DBColumn> m_types;

       public:
        DBTableProto(const std::string& name = "");

        DBTableProto& add(const std::string& name, DBDataType type, DBMOD mod = 0, DBFLAGS flags = DBF_None);
        DBTableProto& add(const std::string& name, DBDataType type, DBMOD mod, DBFLAGS flags,
                          const DBCell& defaultValue);

        // Mark a single column as primary key (by name or index)
        OpRes setPrimaryKey(const std::string& name);
        OpRes setPrimaryKey(size_t index);

        int primaryKeyIndex() const;
        int columnIndex(const std::string& name) const;
        OpRes renameColumn(const std::string& oldName, const std::string& newName);

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

        DBTableBlock& addColumn(const std::string& name, DBDataType type, DBMOD mod = 0,
                                DBFLAGS flags = DBF_None);
        DBTableBlock& addColumn(const std::string& name, DBDataType type, DBMOD mod, DBFLAGS flags,
                                const DBCell& defaultValue);

        std::string toString() const;

        const DBRow& operator[](size_t pos) const;
        DBRow& operator[](size_t pos);

        bool operator==(const DBTableBlock& other) const;
        bool operator!=(const DBTableBlock& other) const;

        Iterator<DBRow> begin();
        Iterator<DBRow> end();

        ConstIterator<DBRow> begin() const;
        ConstIterator<DBRow> end() const;
    };
}  // namespace crb

#endif  // CERBERUS_DBDATA_H
