#ifndef CERBERUS_DATA_DATABASE_SQLDATA_H
#define CERBERUS_DATA_DATABASE_SQLDATA_H

/*  The SQLBlock data class represents a group of rows, aka "block" of rows.
 *  It is used to obtain a result from a query or to insert data into a table.
 *  The SQLBlock does not necessarily have to be structured. The structure property of a block
 *  provides types-convertion functionalities whereas non-structured blocks can contain data anyway
 *
 *  The SQLRow data class represents a single row of a table
 *
 *  The SQLTablePrototype data class represents a structure of a table and contains no data
 *
 */

#include <cstddef>
#include <iterator>
#include <string>
#include <vector>

namespace cerberus
{
    namespace data
    {
        namespace database
        {
            class SQLDatabase;

            class SQLCell
            {
                private:
                    std::string m_value;

                public:
                    SQLCell() = default;

                    SQLCell(double value);
                    SQLCell(float value);
                    SQLCell(bool value);
                    SQLCell(const std::vector<bool>& value);
                    SQLCell(int64_t value);
                    SQLCell(int value);
                    SQLCell(unsigned int value);
                    SQLCell(const std::string& raw);
                    SQLCell(const char* value);

                    void set(const std::string& value);
                    void set(int64_t value);
                    void set(double value);
                    void set(bool value);
                    void set(const std::vector<bool>& value);

                    std::string raw();

                    int64_t toInt();

                    double toFloat();

                    bool toBool();

                    std::vector<bool> toBits();

                    bool isEqual(const SQLCell& other);

                    bool isEqual(const std::string& str);

            };

            class SQLRow
            {
                private:
                    std::vector<SQLCell> m_values;

                public:
                    struct RowIterator
                    {
                            using iterator_category = std::forward_iterator_tag;
                            using difference_type   = std::ptrdiff_t;
                            using value_type        = SQLCell;
                            using pointer           = value_type*;
                            using reference         = value_type&;

                            RowIterator(pointer ptr) : m_ptr(ptr) {}

                            reference operator*() const
                            {
                                return *m_ptr;
                            }
                            pointer operator->()
                            {
                                return m_ptr;
                            }

                            // Prefix increment
                            RowIterator& operator++()
                            {
                                m_ptr++;
                                return *this;
                            }

                            // Postfix increment
                            RowIterator operator++(int)
                            {
                                RowIterator tmp = *this;
                                ++(*this);
                                return tmp;
                            }

                            friend bool operator== (const RowIterator& a, const RowIterator& b)
                            {
                                return a.m_ptr == b.m_ptr;
                            };
                            friend bool operator!= (const RowIterator& a, const RowIterator& b)
                            {
                                return a.m_ptr != b.m_ptr;
                            };


                        private:

                            pointer m_ptr;
                    };

                    SQLRow() = default;

                    SQLRow(const SQLRow& other) = default;

                    ~SQLRow();

                    SQLRow& operator= (const SQLRow& other);

                    void append(const SQLCell& value);

                    size_t size() const;

                    void clear();

                    SQLCell operator [](size_t pos) const;

                    RowIterator begin();

                    RowIterator end();
            };

            class SQLTablePrototype
            {
                public:
                    enum SQLDataType
                    {
                        SDT_Undefined = 0,
                        SDT_BigInt,     //8 byte signed integer
                        SDT_Int,        //4 byte signed integer
                        SDT_SmallInt,   //2 byte signed integer
                        SDT_Real,       //4 byte signed float
                        SDT_Double,     //8 byte signed float
                        SDT_Boolean,    //bool
                        SDT_Bit,        //fixed length bit array
                        SDT_VarBit,     //variable length bit array
                        SDT_Char,       //fixed length char array
                        SDT_VarChar,    //variable length char array
                        SDT_Money,      //fixed fractional precision (2 digits typically)
                    };

                    static SQLDataType toSQLDataType(const std::string& type);

                    static std::string fromSQLDataType(SQLDataType type);

                    struct SQLColumn
                    {
                        public:
                            SQLColumn() = delete;
                            SQLColumn(const std::string& name, SQLDataType type, int mod = -1): m_columnName(name), m_type(type), m_mod(mod) {};

                            std::string name()
                            {
                                return m_columnName;
                            };

                            SQLDataType type()
                            {
                                return m_type;
                            };

                            int mod()
                            {
                                return m_mod;
                            };

                            std::string typeString()
                            {
                                return fromSQLDataType(m_type);
                            };

                        private:
                            std::string m_columnName;
                            SQLDataType m_type;
                            int m_mod;
                    };

                private:
                    std::string m_name; //the table name

                    std::vector<SQLColumn> m_types;

                public:
                    struct PrototypeIterator
                    {
                            using iterator_category = std::forward_iterator_tag;
                            using difference_type   = std::ptrdiff_t;
                            using value_type        = SQLColumn;
                            using pointer           = value_type*;
                            using reference         = value_type&;

                            PrototypeIterator(pointer ptr) : m_ptr(ptr) {}

                            reference operator*() const
                            {
                                return *m_ptr;
                            }
                            pointer operator->()
                            {
                                return m_ptr;
                            }

                            // Prefix increment
                            PrototypeIterator& operator++()
                            {
                                m_ptr++;
                                return *this;
                            }

                            // Postfix increment
                            PrototypeIterator operator++(int)
                            {
                                PrototypeIterator tmp = *this;
                                ++(*this);
                                return tmp;
                            }

                            friend bool operator== (const PrototypeIterator& a, const PrototypeIterator& b)
                            {
                                return a.m_ptr == b.m_ptr;
                            };
                            friend bool operator!= (const PrototypeIterator& a, const PrototypeIterator& b)
                            {
                                return a.m_ptr != b.m_ptr;
                            };


                        private:

                            pointer m_ptr;
                    };

                    SQLTablePrototype() = delete;

                    SQLTablePrototype(const std::string& name);

                    SQLTablePrototype& add(const std::string& name, SQLDataType type, int mod = -1);

                    SQLColumn operator[](int index) const;

                    void clear();

                    size_t size() const;

                    std::string name() const;

                    PrototypeIterator begin();

                    PrototypeIterator end();
            };

            class SQLBlock
            {
                    friend class cerberus::data::database::SQLDatabase;

                private:
                    bool m_failed;

                    std::string m_failureReason;

                    std::vector<SQLRow> m_rows;

                    SQLTablePrototype m_prototype;

                public:
                    struct BlockIterator
                    {
                            using iterator_category = std::forward_iterator_tag;
                            using difference_type   = std::ptrdiff_t;
                            using value_type        = SQLRow;
                            using pointer           = value_type*;
                            using reference         = value_type&;

                            BlockIterator(pointer ptr) : m_ptr(ptr) {}

                            reference operator*() const
                            {
                                return *m_ptr;
                            }
                            pointer operator->()
                            {
                                return m_ptr;
                            }

                            // Prefix increment
                            BlockIterator& operator++()
                            {
                                m_ptr++;
                                return *this;
                            }

                            // Postfix increment
                            BlockIterator operator++(int)
                            {
                                BlockIterator tmp = *this;
                                ++(*this);
                                return tmp;
                            }

                            friend bool operator== (const BlockIterator& a, const BlockIterator& b)
                            {
                                return a.m_ptr == b.m_ptr;
                            };
                            friend bool operator!= (const BlockIterator& a, const BlockIterator& b)
                            {
                                return a.m_ptr != b.m_ptr;
                            };


                        private:

                            pointer m_ptr;
                    };

                    SQLBlock();

                    SQLBlock(const std::string& name);

                    SQLBlock(const SQLBlock& other) = default;

                    ~SQLBlock();

                    bool isFailed() const;

                    std::string failureReason() const;

                    bool append(const SQLRow& row);

                    size_t size() const;

                    bool empty() const;

                    void clear();

                    bool structured();

                    void clearStructure();

                    void clearRows();

                    void setPrototype(const SQLTablePrototype& prototype);

                    SQLTablePrototype prototype() const;

                    SQLBlock& addColumn(const std::string& name, SQLTablePrototype::SQLDataType type, int mod = -1);

                    SQLRow operator[](size_t pos) const;

                    bool operator== (const SQLBlock& other)const;

                    bool operator!= (const SQLBlock& other)const;

                    BlockIterator begin();

                    BlockIterator end();
            };
        }
    }
}

#endif // CERBERUS_DATA_DATABASE_SQLDATA_H
