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

        CERBERUS_EXPORT DBCell(int64_t value);
        CERBERUS_EXPORT DBCell(float value);
        CERBERUS_EXPORT DBCell(long double value);
        CERBERUS_EXPORT DBCell(bool value);
        CERBERUS_EXPORT DBCell(const std::vector<bool>& value);
        CERBERUS_EXPORT DBCell(const std::string& str);
        CERBERUS_EXPORT DBCell(const char* str);
        CERBERUS_EXPORT DBCell(const ByteBuffer& raw);

        void set(const std::string& str);
        void set(const char* str);
        void set(int64_t value);
        void set(long double value);
        void set(bool value);
        void set(const std::vector<bool>& value);

        CERBERUS_EXPORT ByteBuffer& raw();
        CERBERUS_EXPORT const ByteBuffer& raw() const;

        ByteBuffer serialize(DBDataType type, DBMOD mod) const;

        CERBERUS_EXPORT int64_t toInt() const;
        CERBERUS_EXPORT long double toReal() const;
        CERBERUS_EXPORT bool toBool() const;
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
        explicit CERBERUS_EXPORT DBQuery(const std::string& tableName);

        const std::string& table() const;
        void setTable(const std::string& tableName);

        const DBQueryCondition& condition() const;
        CERBERUS_EXPORT DBQueryCondition& condition();

        const std::vector<std::string>& columns() const;
        void setColumns(const std::vector<std::string>& columns);
        CERBERUS_EXPORT void addColumn(const std::string& column);
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
        CERBERUS_EXPORT DBRow();

        CERBERUS_EXPORT DBRow(const DBRow& other);

        CERBERUS_EXPORT DBRow& operator=(const DBRow& other);

        DBRow(DBRow&& other) noexcept = default;
        DBRow& operator=(DBRow&& other) noexcept = default;

        CERBERUS_EXPORT ~DBRow();

        void bindPrototype(const DBTableProto* proto);
        void detachPrototype();
        bool hasPrototype() const;
        const DBTableProto* prototype() const;

        CERBERUS_EXPORT void append(const DBCell& value);
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

        CERBERUS_EXPORT void clear();

        CERBERUS_EXPORT bool verify(const DBTableProto& proto) const;

        ByteBuffer serialize(const DBTableProto& proto) const;

        CERBERUS_EXPORT const DBCell& operator[](size_t pos) const;
        CERBERUS_EXPORT DBCell& operator[](size_t pos);
        CERBERUS_EXPORT const DBCell& operator[](const std::string& column) const;
        CERBERUS_EXPORT DBCell& operator[](const std::string& column);

        CERBERUS_EXPORT Iterator<DBCell> begin();
        CERBERUS_EXPORT Iterator<DBCell> end();

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
        CERBERUS_EXPORT DBTableProto(const std::string& name = "");

        CERBERUS_EXPORT DBTableProto& add(const std::string& name, DBDataType type, DBMOD mod = 0,
                                          DBFLAGS flags = DBF_None);
        CERBERUS_EXPORT DBTableProto& add(const std::string& name, DBDataType type, DBMOD mod,
                                          DBFLAGS flags, const DBCell& defaultValue);

        // Mark a single column as primary key (by name or index)
        OpRes setPrimaryKey(const std::string& name);
        OpRes setPrimaryKey(size_t index);

        int primaryKeyIndex() const;
        CERBERUS_EXPORT int columnIndex(const std::string& name) const;
        OpRes renameColumn(const std::string& oldName, const std::string& newName);

        CERBERUS_EXPORT const DBColumn& operator[](int index) const;
        CERBERUS_EXPORT DBColumn& operator[](int index);

        void clear();

        CERBERUS_EXPORT size_t size() const;

        CERBERUS_EXPORT bool isEqual(const DBTableProto& other) const;

        std::string name() const;
        void setName(const std::string& name);

        CERBERUS_EXPORT Iterator<DBColumn> begin();
        CERBERUS_EXPORT Iterator<DBColumn> end();

        CERBERUS_EXPORT ConstIterator<DBColumn> begin() const;
        CERBERUS_EXPORT ConstIterator<DBColumn> end() const;
    };

    class DBTableBlock
    {
        std::vector<DBRow> m_rows;

        DBTableProto m_prototype;

       public:
        DBTableBlock() = default;

        DBTableBlock(const DBTableBlock& other) = default;

        CERBERUS_EXPORT DBTableBlock(const std::string& name);

        CERBERUS_EXPORT DBTableBlock(const DBTableProto& proto);

        DBTableBlock(const DBRow& row);

        CERBERUS_EXPORT ~DBTableBlock();

        CERBERUS_EXPORT OpRes append(const DBRow& row);

        CERBERUS_EXPORT size_t size() const;

        CERBERUS_EXPORT bool empty() const;

        void clear();

        CERBERUS_EXPORT bool structured() const;

        void clearStructure();

        void clearRows();

        CERBERUS_EXPORT void setPrototype(const DBTableProto& prototype);

        void assignRows(const DBTableBlock& other);

        bool verify() const;

        CERBERUS_EXPORT bool verify(const DBTableProto& proto) const;

        ByteBuffer serialize(const DBTableProto& proto) const;

        CERBERUS_EXPORT const DBTableProto& prototype() const;

        DBTableBlock& addColumn(const std::string& name, DBDataType type, DBMOD mod = 0,
                                DBFLAGS flags = DBF_None);
        DBTableBlock& addColumn(const std::string& name, DBDataType type, DBMOD mod, DBFLAGS flags,
                                const DBCell& defaultValue);

        CERBERUS_EXPORT std::string toString() const;

        const DBRow& operator[](size_t pos) const;
        CERBERUS_EXPORT DBRow& operator[](size_t pos);

        bool operator==(const DBTableBlock& other) const;
        bool operator!=(const DBTableBlock& other) const;

        CERBERUS_EXPORT Iterator<DBRow> begin();
        CERBERUS_EXPORT Iterator<DBRow> end();

        CERBERUS_EXPORT ConstIterator<DBRow> begin() const;
        CERBERUS_EXPORT ConstIterator<DBRow> end() const;
    };
}  // namespace crb

#endif  // CERBERUS_DBDATA_H
