#ifndef CERBERUS_DATA_FILESYSTEM_FILE_H
#define CERBERUS_DATA_FILESYSTEM_FILE_H

#include <string>
#include <fstream>
#include "../../Cerberus_global.h"
#include "../../define.h"

namespace cerberus
{
    namespace data
    {
        class ByteBuffer;

        namespace filesystem
        {
            class CERBERUS_EXPORT File
            {
                private:
                    std::string m_fileName;

                    std::fstream m_stream;

                    std::ios_base::openmode m_openMode;

                public:
                    static bool exist(const std::string& fileName);

                    File(uint8_t openMode = 0); //Default: read-only

                    File(const std::string& fileName, uint8_t openMode = 0);

                    ~File();

                    void setFileName(const std::string& fileName);

                    void setOpenMode(uint8_t openMode = 0);

                    bool isOpen() const;

                    bool open();

                    bool close();

                    bool deleteFromDisk();

                    bool rename(const std::string& newName);

                    bool write(const ByteBuffer& bytes);

                    bool writeLine(const std::string& line);

                    void read(ByteBuffer& bytes, std::streampos start = 0);

                    void read(ByteBuffer& bytes, std::streampos start, std::streamsize span);

                    //Returns false when EOF is reached and no more lines are available
                    bool readLine(std::string& line);

                    void resetReadCursor();

                    void resetWriteCursor();

                    std::streampos readCursor();

                    std::streampos writeCursor();

                    void setReadCursor(std::streampos pos);

                    void setWriteCursor(std::streampos pos);

                    void moveReadCursor(std::streamoff offset);

                    void moveWriteCursor(std::streamoff offset);
            };
        }
    }
}

#endif // CERBERUS_DATA_FILESYSTEM_FILE_H
