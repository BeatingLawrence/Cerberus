#ifndef FILESYSTEMDB_H
#define FILESYSTEMDB_H

#include "../../data/filesystem/file.h"
#include "idatabase.h"

namespace crb
{
    namespace db
    {
        class FilesystemDB : public IDatabase
        {
            struct Table
            {
                HASH32 tableID;
                DBTableProto proto;
                LSIZE header;  // position in file where the table header begins
                LSIZE buffer;  // position in file where the table data begins
                bool hasDupKey;

                Table()
                    : tableID(0),
                      proto(),
                      header(0),
                      buffer(0),
                      hasDupKey(false) {};

                Table(HASH32 id, const DBTableProto& proto, LSIZE h, LSIZE b)
                    : tableID(id),
                      proto(proto),
                      header(h),
                      buffer(b),
                      hasDupKey(false) {};
            };

            struct Buf_Size
            {
                ByteBuffer buf;
                LSIZE size;
            };

            struct ParsedQuery
            {
                std::string table;
                std::string pattern;
                std::string columns;
            };

            struct Pattern
            {
                enum Type
                {
                    Range,
                    Exact,
                    Regex
                } type;
                std::string col;
                std::optional<long double> min;
                std::optional<long double> max;
                std::string exactStr;
            };

            File m_file;

            std::vector<Table> m_tables;

            StringOpRes _readStr();

            DBMOD _getMod(const ByteBuffer& buf);

            OpRes _parseTableHeader(Table& tab);

            OpRes _load();

            OpRes _buildHeader(const DBTableProto& prototype, Table& tab);

            OpResData<Buf_Size> _parseFieldRaw(DBDataType type, DBMOD mod);

            OpResData<DBCell> _parseField(DBDataType type, DBMOD mod);

            OpRes _skipTable(Table& tab, LSIZE* endPos);

            OpResData<DBTableBlock> _getTable(Table& tab);

            OpRes _insertBlock(Table* tab, const DBTableBlock& block);

            ParsedQuery _splitQuery(const std::string& query) const;
            int _columnIndex(const DBTableProto& proto, const std::string& name) const;
            std::vector<int> _parseColumns(const std::vector<std::string>& columns, const DBTableProto& proto,
                                           DBTableProto& outProto) const;

            std::string _cellToString(const DBCell& cell, DBDataType t) const;

            bool _matchRow(const DBQueryCondition& cond, int colIndex, const DBRow& row,
                           const DBTableProto& proto) const;

           public:
            FilesystemDB();

            virtual OpRes init(const std::string& parameters);

            virtual void deinit();

            virtual bool ready() const;

            virtual OpRes command(const string& query);

            virtual OpResData<DBTableBlock> queryBlock(const string& query);
            virtual OpResData<DBTableBlock> queryBlock(const DBQuery& query);

            virtual OpResData<DBTableProto> queryPrototype(const string& tableName);

            virtual OpRes createTable(const DBTableProto& prototype);

            virtual OpRes insertBlock(const DBTableBlock& block);

            virtual OpRes updateBlock(const DBTableBlock& block, UpdatePolicy policy = UP_UpdateInsert);

            virtual OpRes dropTable(const std::string& table);
            virtual OpRes renameColumn(const std::string& table, const std::string& oldName,
                                       const std::string& newName);

            inline virtual OpResData<DBTableBlock> querytable(const std::string& tableName);

            virtual ~FilesystemDB();
        };
    }  // namespace db
}  // namespace crb

#endif  // FILESYSTEMDB_H
