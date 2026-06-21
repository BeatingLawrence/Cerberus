#include "csvdatafile.h"

#include <algorithm>

#include "../../core/cerberusutils.h"
#include "../bytebuffer.h"

using namespace crb;

namespace
{
    const char kDelimiters[] = {',', ';', '\t', '|'};

    inline bool isKnownDelimiter(char c)
    {
        for (char d : kDelimiters)
            if (c == d) return true;
        return false;
    }

    struct ParsedRow
    {
        MultiString fields;
        LSIZE nextPos{0};        // index of first char after the row
        size_t newlineLen{0};    // 0 if EOF, 1 for \n or \r, 2 for \r\n
        bool multiDelim{false};  // true if another delimiter (different from current) has been found
        bool malformed{false};   // true if quotes are unbalanced
        bool eof{false};         // true if EOF was reached while reading this row
    };

    // Reads a CSV row starting at file position 'start'. Supports quotes and escaped quotes ("").
    // If allowInferDelim is true and delim == 0, the first delimiter encountered becomes delim.
    // Returns OR_OK, OR_EOF or failure codes.
    OpRes parseRowFromFile(const File& file, LSIZE start, char& delim, bool allowInferDelim, ParsedRow& out)
    {
        out = ParsedRow{};

        auto resSeek = file.seek(start);
        if (resSeek.fail()) return resSeek;

        bool inQuotes = false;
        std::string current;
        LSIZE pos = start;

        while (true)
        {
            ByteBuffer b;
            auto r = file.readChunk(b, 1);

            if (r.res == OR_EOF)
            {
                out.fields.push_back(current);
                out.nextPos = pos;
                out.eof     = true;
                return OR_OK;
            }

            if (r.fail()) return r;

            char c = b[0];
            ++pos;

            if (inQuotes)
            {
                if (c == '"')
                {
                    // peek next char to check for escaped quote
                    ByteBuffer peek;
                    auto r2 = file.readChunk(peek, 1);

                    if (r2.res == OR_EOF)
                    {
                        out.fields.push_back(current);
                        out.nextPos = pos;
                        out.eof     = true;
                        return OR_OK;
                    }

                    if (r2.fail()) return r2;

                    char nextc = peek[0];
                    ++pos;

                    if (nextc == '"')
                    {
                        current.push_back('"');
                        continue;  // still in quotes
                    }

                    // end of quotes, unread the peeked char
                    file.seekOffset(-1);
                    --pos;
                    inQuotes = false;
                    continue;
                }

                current.push_back(c);
                continue;
            }

            // not in quotes
            if (c == '"')
            {
                inQuotes = true;
                continue;
            }

            if (allowInferDelim && delim == 0 && isKnownDelimiter(c))
            {
                delim = c;
                out.fields.push_back(current);
                current.clear();
                continue;
            }

            if (c == delim)
            {
                out.fields.push_back(current);
                current.clear();
                continue;
            }

            if (isKnownDelimiter(c) && c != delim && !allowInferDelim)
            {
                out.multiDelim = true;
                out.nextPos    = pos;
                return OR_OK;
            }

            if ((c == '\n' || c == '\r') && !inQuotes)
            {
                out.fields.push_back(current);
                out.newlineLen = 1;

                if (c == '\r')
                {
                    ByteBuffer peek;
                    auto r2 = file.readChunk(peek, 1);
                    if (!r2.fail() && peek.size() > 0 && peek[0] == '\n')
                    {
                        out.newlineLen = 2;
                        ++pos;
                    }
                    else
                    {
                        // unread if not part of CRLF
                        if (!r2.fail())
                        {
                            file.seekOffset(-1);
                            --pos;
                        }
                    }
                }

                out.nextPos = pos;
                return OR_OK;
            }

            current.push_back(c);
        }
    }

    std::string escapeField(const std::string& field, char delim)
    {
        bool needsQuotes = field.find_first_of(std::string() + delim + "\r\n\"") != std::string::npos;

        if (!needsQuotes) return field;

        std::string out;
        out.reserve(field.size() + 2);
        out.push_back('"');
        for (char c : field)
        {
            if (c == '"') out.push_back('"');  // escape by doubling
            out.push_back(c);
        }
        out.push_back('"');
        return out;
    }

    std::string stringifyRecord(const CSVDataFile::CSVRecord& rec, char delim)
    {
        std::string line;
        for (size_t i = 0; i < rec.record.size(); ++i)
        {
            if (i) line.push_back(delim);
            line.append(escapeField(rec.record[i].get(), delim));
        }
        return line;
    }

    OpRes copyRange(File& src, File& dst, LSIZE start, LSIZE end)
    {
        auto res = src.seek(start);
        if (res.fail()) return res;

        LSIZE remaining = end - start;

        while (remaining > 0)
        {
            ByteBuffer b;
            auto r = src.readChunk(b, 1);
            if (r.res == OR_EOF) break;
            if (r.fail()) return r;
            --remaining;
            condret(dst.write(b));
        }

        return OR_OK;
    }

    OpRes copyToEOF(File& src, File& dst)
    {
        while (true)
        {
            ByteBuffer b;
            auto r = src.readChunk(b, 1);
            if (r.res == OR_EOF) break;
            if (r.fail()) return r;
            condret(dst.write(b));
        }
        return OR_OK;
    }
}  // namespace

//=============================================================================
void CSVDataFile::CSVRecord::addValue(const Opaque& value) { record.push_back(value); }
//=============================================================================
CSVDataFile::CSVDataFile(const std::string& fileName, char delim)
    : m_file(fileName, FOM_ReadWrite),
      m_delim(delim)
{
    if (!fileName.empty()) m_file.open();  // keep open if possible
}
//=============================================================================
void CSVDataFile::setFileName(const std::string& fileName, char delim)
{
    m_file.close();
    m_file.path(fileName);
    m_file.setOpenMode(FOM_ReadWrite);
    m_delim  = delim;
    auto res = m_file.open();
    if (res.fail()) throw cIllegalStateExc("Failed to open CSV file: %s", res.toStr().c_str());
}
//=============================================================================
void CSVDataFile::close()
{
    m_file.close();
    m_columns.clear();
    m_columnTypes.clear();
    m_records.clear();
    m_delim = 0;
}
//=============================================================================
OpRes CSVDataFile::load()
{
    if (m_file.path().empty()) return OR_InvalidPath;

    m_records.clear();
    m_columns.clear();
    m_columnTypes.clear();

    if (m_file.isOpen()) m_file.close();
    m_file.setOpenMode(FOM_ReadWrite);
    condret(m_file.open());
    m_file.seek(0);

    LSIZE pos  = 0;
    char delim = m_delim;

    ParsedRow header;
    condret(parseRowFromFile(m_file, pos, delim, true, header));

    if (header.malformed || header.multiDelim || delim == 0 || header.fields.empty()) return OR_InvalidFile;

    m_columns = header.fields;
    pos       = header.nextPos;
    m_delim   = delim;

    // first data row defines column types
    if (header.eof) return OR_InvalidFile;  // no data rows present

    ParsedRow firstRow;
    condret(parseRowFromFile(m_file, pos, delim, false, firstRow));
    if (firstRow.malformed || firstRow.multiDelim) return OR_InvalidFile;
    if (firstRow.fields.size() != m_columns.size()) return OR_InvalidFile;

    for (auto&& field : firstRow.fields)
    {
        Opaque v(field);
        auto t = v.type();
        if (t == DT_Invalid) return OR_InvalidFile;
        m_columnTypes.push_back(t);
    }

    m_records.push_back(pos);
    pos = firstRow.nextPos;

    bool optFail  = false;
    auto filesize = m_file.size();
    condret(filesize);

    while (pos < filesize.value)
    {
        ParsedRow row;
        auto r = parseRowFromFile(m_file, pos, delim, false, row);
        if (r.res == OR_EOF || row.eof)
        {
            break;
        }
        if (r.fail()) return r;

        if (row.malformed || row.multiDelim) return OR_InvalidFile;
        if (row.fields.size() != m_columns.size())
        {
            optFail = true;
            pos     = row.nextPos;
            continue;
        }

        bool valid = true;
        for (size_t i = 0; i < row.fields.size(); ++i)
        {
            Opaque v(row.fields[i]);
            if (v.type() != m_columnTypes[i])
            {
                valid = false;
                break;
            }
        }

        if (valid)
        {
            m_records.push_back(pos);
        }
        else
        {
            optFail = true;
        }

        pos = row.nextPos;
    }

    OpRes out = OR_OK;
    if (optFail) out.addOptional(OR_Failure);
    return out;
}
//=============================================================================
crb::LSIZE CSVDataFile::size() const { return m_records.size(); }
//=============================================================================
int CSVDataFile::columnPos(const std::string& col) const
{
    for (size_t i = 0; i < m_columns.size(); ++i)
        if (m_columns[i] == col) return static_cast<int>(i);
    return INVALID_COLUMN;
}
//=============================================================================
MultiString CSVDataFile::columns() const { return m_columns; }
//=============================================================================
DataType CSVDataFile::type(crb::SIZE columnIndex) const
{
    if (columnIndex >= m_columnTypes.size()) return DT_Invalid;
    return m_columnTypes[columnIndex];
}
//=============================================================================
static ParsedRow parseRowAt(const File& file, LSIZE start, char delim)
{
    ParsedRow row;
    char d = delim;
    parseRowFromFile(file, start, d, false, row);
    return row;
}
//=============================================================================
Opaque CSVDataFile::read(crb::LSIZE recordIndex, crb::SIZE columnIndex) const
{
    if (recordIndex >= m_records.size()) throw cIllegalArgExc("Record index out of range");
    if (columnIndex >= m_columns.size()) throw cIllegalArgExc("Column index out of range");

        auto row = parseRowAt(m_file, m_records[recordIndex], m_delim);
    if (row.fields.size() <= columnIndex) throw cIllegalStateExc("Column missing in record");

    return Opaque(row.fields[columnIndex]);
}
//=============================================================================
static void validateRecord(const CSVDataFile::CSVRecord& record, const MultiString& columns,
                           const std::vector<DataType>& types)
{
    if (record.record.size() != columns.size()) throw cIllegalArgExc("Record has wrong column count");

    for (size_t i = 0; i < record.record.size(); ++i)
    {
        if (record.record[i].type() != types[i])
            throw cIllegalArgExc("Record field type mismatch at column %zu", i);
    }
}

static OpRes rewriteSegment(File& src, LSIZE start, LSIZE end, const std::string& replacement)
{
    // create temp file in the same directory as src to avoid permission issues
    File tmp = File::tmpFile(src.directory(), FOM_ReadWrite);

    // copy 0 .. start
    condret(copyRange(src, tmp, 0, start));

    // write replacement
    ByteBuffer repBuf(replacement);
    condret(tmp.write(repBuf));

    // skip original segment and copy rest
    condret(src.seek(end));
    condret(copyToEOF(src, tmp));

    tmp.close();
    src.close();

    auto mv = File::move(tmp.path(), src.path());
    if (mv.fail()) return mv;

    src.setOpenMode(FOM_ReadWrite);
    return src.open();
}
//=============================================================================
void CSVDataFile::write(crb::LSIZE recordIndex, crb::SIZE columnIndex, const Opaque& value)
{
    if (recordIndex >= m_records.size()) throw cIllegalArgExc("Record index out of range");
    if (columnIndex >= m_columns.size()) throw cIllegalArgExc("Column index out of range");
    if (value.type() != m_columnTypes[columnIndex]) throw cIllegalArgExc("Type mismatch when writing record");

    auto row = parseRowAt(m_file, m_records[recordIndex], m_delim);
    if (row.fields.size() != m_columns.size()) throw cIllegalStateExc("Cached record is malformed");

    row.fields[columnIndex] = value.get();

    CSVRecord rec;
    rec.record.reserve(row.fields.size());
    for (auto&& f : row.fields) rec.record.emplace_back(f);

    std::string newLine = stringifyRecord(rec, m_delim);
    newLine.append(row.newlineLen == 2 ? "\r\n" : "\n");

    LSIZE oldLen   = row.nextPos - m_records[recordIndex];
    LSIZE newLen   = newLine.size();
    LSIZE oldStart = m_records[recordIndex];
    LSIZE oldEnd   = row.nextPos;

    auto res = rewriteSegment(m_file, oldStart, oldEnd, newLine);
    if (res.fail()) throw cIllegalStateExc("Failed to write CSV file: %s", res.toStr().c_str());

    OFFSET diff = static_cast<OFFSET>(newLen) - static_cast<OFFSET>(oldLen);
    for (size_t i = recordIndex + 1; i < m_records.size(); ++i) m_records[i] += diff;
}
//=============================================================================
void CSVDataFile::addRecord(const CSVRecord& record)
{
    if (m_columns.empty()) throw cIllegalStateExc("File not loaded");
    validateRecord(record, m_columns, m_columnTypes);

    auto sz = m_file.size();
    if (sz.fail()) throw cIllegalStateExc("Cannot stat CSV file: %s", sz.toStr().c_str());

    LSIZE insertPos = sz.value;

    std::string line = stringifyRecord(record, m_delim);
    line.push_back('\n');

    m_file.seekToEOF();
    ByteBuffer buf(line);
    auto resw = m_file.write(buf);
    if (resw.fail()) throw cIllegalStateExc("Failed to append CSV: %s", resw.toStr().c_str());

    m_records.push_back(insertPos);
}
//=============================================================================
void CSVDataFile::addRecordBlock(const CSVRecordBlock& block)
{
    for (auto&& rec : block) addRecord(rec);
}
//=============================================================================
void CSVDataFile::insertRecord(const CSVRecord& record, crb::LSIZE recordIndex)
{
    if (recordIndex > m_records.size()) throw cIllegalArgExc("Record index too large");
    validateRecord(record, m_columns, m_columnTypes);

    std::string line = stringifyRecord(record, m_delim);
    line.push_back('\n');

    if (recordIndex == m_records.size())
    {
        addRecord(record);
        return;
    }

    LSIZE pos = m_records[recordIndex];

    auto res = rewriteSegment(m_file, pos, pos, line);
    if (res.fail()) throw cIllegalStateExc("Failed to insert record: %s", res.toStr().c_str());

    m_records.insert(m_records.begin() + static_cast<std::vector<LSIZE>::difference_type>(recordIndex), pos);
    for (size_t i = recordIndex + 1; i < m_records.size(); ++i) m_records[i] += line.size();
}
//=============================================================================
void CSVDataFile::insertRecordBlock(const CSVRecordBlock& block, crb::LSIZE recordIndex)
{
    crb::LSIZE idx = recordIndex;
    for (auto&& rec : block)
    {
        insertRecord(rec, idx);
        ++idx;
    }
}
//=============================================================================
CSVDataFile::CSVRecord CSVDataFile::getRecord(crb::LSIZE recordIndex)
{
    if (recordIndex >= m_records.size()) throw cIllegalArgExc("Record index out of range");

    auto row = parseRowAt(m_file, m_records[recordIndex], m_delim);
    CSVRecord rec;
    rec.record.reserve(row.fields.size());
    for (auto&& f : row.fields) rec.record.emplace_back(f);
    return rec;
}
//=============================================================================
CSVDataFile::CSVRecordBlock CSVDataFile::getRecordBlock(crb::LSIZE recordIndex, crb::LSIZE span)
{
    if (recordIndex >= m_records.size()) throw cIllegalArgExc("Record index out of range");

    CSVRecordBlock out;
    crb::LSIZE end = span ? std::min<crb::LSIZE>(m_records.size(), recordIndex + span) : m_records.size();

    out.reserve(end - recordIndex);
    for (crb::LSIZE i = recordIndex; i < end; ++i) out.push_back(getRecord(i));
    return out;
}
//=============================================================================
CSVDataFile::MultiVal CSVDataFile::getColumn(crb::SIZE index)
{
    if (index >= m_columns.size()) throw cIllegalArgExc("Column index out of range");

    MultiVal out;
    out.reserve(m_records.size());

    for (auto pos : m_records)
    {
        auto row = parseRowAt(m_file, pos, m_delim);
        if (row.fields.size() <= index) continue;  // skip malformed cached rows just in case
        out.emplace_back(row.fields[index]);
    }

    return out;
}
//=============================================================================
CSVDataFile::MultiVal CSVDataFile::getColumn(const std::string& headerName)
{
    int idx = columnPos(headerName);
    if (idx == INVALID_COLUMN) throw cIllegalArgExc("Column not found");
    return getColumn(static_cast<crb::SIZE>(idx));
}
