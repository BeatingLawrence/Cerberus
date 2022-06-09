#include "file.h"
#include "../../exception/exceptioncatalog.h"
#include "../../core/cerberusutils.h"
#include "../../core/cerberuslog.h"
#include "../bytebuffer.h"
#include <cstdio>
#include <regex>
#include <string.h>

#ifdef WINDOWS_SYSTEM
    #include <windows.h>
#else
    #include <unistd.h>
    #include <sys/stat.h>
    #include <dirent.h>
#endif

//=============================================================================
bool cerberus::data::filesystem::File::existsAsFile(const std::string& path)
{
    if(path.empty())
    {
        throw cerberusIllegalArgumentExc("Path is empty");
    }

#ifdef WINDOWS_SYSTEM

    if(GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES) //TODO: true if is a file only
    {
        exists = true;
    }

#else
    struct stat stat_struct;
    int ret = stat(path.c_str(), &stat_struct);
    logInfo("%i", ret);

    if(ret == 0)
    {
        if(S_ISREG(stat_struct.st_mode))
        {
            return true;
        }

        return false;
    }
    else
    {
        if(errno == ENOENT)
        {
            return false;
        }
        else
        {
            throw cerberusSystemExc("stat error: %s", strerror(errno));
        }
    }

#endif
}
//=============================================================================
bool cerberus::data::filesystem::File::existsAsDirectory(const std::string& path)
{
#ifdef WINDOWS_SYSTEM
    throw cerberusImplementationMissExc("DIRECTORY EXISTANCE CHECK NOT IMPLEMENTED YET");

    if(GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES) //TODO: true if is a directory only
    {
        //
    }

#else
    struct stat stat_struct;
    int ret = stat(path.c_str(), &stat_struct);
    logInfo("%i", ret);

    if(ret == 0)
    {
        if(S_ISDIR(stat_struct.st_mode))
        {
            return true;
        }

        return false;
    }
    else
    {
        if(errno == ENOENT)
        {
            return false;
        }
        else
        {
            throw cerberusSystemExc("stat error: %s", strerror(errno));
        }
    }

#endif
}
//=============================================================================
void cerberus::data::filesystem::File::createDirectory(const std::string& path)
{
#ifdef WINDOWS_SYSTEM
    throw cerberusImplementationMissExc("DIRECTORY CREATION NOT IMPLEMENTED YET");
#else
    int ret = mkdir(path.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

    if(ret == -1)
    {
        throw cerberusSystemExc("mkdir error: %s", strerror(errno));
    }

#endif
}
//=============================================================================
void cerberus::data::filesystem::File::deleteDirectory(const std::string& path)
{
#ifdef WINDOWS_SYSTEM
    throw cerberusImplementationMissExc("DIRECTORY DELETION NOT IMPLEMENTED YET");
#else
    int ret = rmdir(path.c_str());

    if(ret == -1)
    {
        throw cerberusSystemExc("rmdir error: %s", strerror(errno));
    }

#endif
}
//=============================================================================
bool cerberus::data::filesystem::File::isEmptyDirectory(const std::string& path)
{
#ifdef WINDOWS_SYSTEM
    throw cerberusImplementationMissExc("DIRECTORY EMPTY CHECK NOT IMPLEMENTED YET");
#else
    int n = 0;
    struct dirent* d;
    DIR* dir = opendir(path.c_str());

    if(dir == NULL)
    {
        throw cerberusIllegalArgumentExc("Given directory path is not a directory");
    }

    errno = 0;

    while(readdir(dir) != NULL)
    {
        if(++n > 2)
        {
            break;
        }
    }

    closedir(dir);

    if(errno != 0)
    {
        throw cerberusIllegalStateExc("readdir error: %s", strerror(errno));
    }

    if(n <= 2)
    {
        return true;
    }

    return false;
#endif
}
//=============================================================================
cerberus::data::filesystem::File::File(uint8_t openMode) :
    m_filePath(),
    m_stream(),
    m_openMode(std::ios_base::in)
{
    if(openMode & CERBERUS_FILE_WRITE)
    {
        m_openMode |= std::ios_base::out;
    }

    if(openMode & CERBERUS_FILE_BINARY)
    {
        m_openMode |= std::ios_base::binary;
    }

    if(openMode & CERBERUS_FILE_EOF)
    {
        m_openMode |= std::ios_base::ate;
    }

    if(openMode & CERBERUS_FILE_APPEND)
    {
        m_openMode |= std::ios_base::app;
    }

    if(openMode & CERBERUS_FILE_TRUNCATE)
    {
        m_openMode |= std::ios_base::trunc;
    }

    m_stream.exceptions(std::ifstream::badbit);    //will throw exception if bad
}
//=============================================================================
cerberus::data::filesystem::File::File(const std::string& filePath, uint8_t openMode) :
    m_filePath(filePath),
    m_stream(),
    m_openMode(std::ios_base::in)
{
    if(filePath.empty())
    {
        throw cerberusIllegalArgumentExc("Filename is empty");
    }

    if(openMode & CERBERUS_FILE_WRITE)
    {
        m_openMode |= std::ios_base::out;
    }

    if(openMode & CERBERUS_FILE_BINARY)
    {
        m_openMode |= std::ios_base::binary;
    }

    if(openMode & CERBERUS_FILE_EOF)
    {
        m_openMode |= std::ios_base::ate;
    }

    if(openMode & CERBERUS_FILE_APPEND)
    {
        m_openMode |= std::ios_base::app;
    }

    if(openMode & CERBERUS_FILE_TRUNCATE)
    {
        m_openMode |= std::ios_base::trunc;
    }

    m_stream.exceptions(std::ifstream::badbit);    //will throw exception if bad
}
//=============================================================================
cerberus::data::filesystem::File::~File()
{
    if(m_stream.is_open())
    {
        m_stream.close();
    }
}
//=============================================================================
void cerberus::data::filesystem::File::setFileName(const std::string& filePath)
{
    if(m_stream.is_open())
    {
        throw cerberusIllegalStateExc("Cannot change a name of an open file");
    }

    m_filePath = filePath;
}
//=============================================================================
void cerberus::data::filesystem::File::setOpenMode(uint8_t openMode)
{
    if(m_stream.is_open())
    {
        throw cerberusIllegalStateExc("Cannot change open mode of an open file");
    }

    m_openMode = std::ios_base::in;

    if(openMode & CERBERUS_FILE_WRITE)
    {
        m_openMode |= std::ios_base::out;
    }

    if(openMode & CERBERUS_FILE_BINARY)
    {
        m_openMode |= std::ios_base::binary;
    }

    if(openMode & CERBERUS_FILE_EOF)
    {
        m_openMode |= std::ios_base::ate;
    }

    if(openMode & CERBERUS_FILE_APPEND)
    {
        m_openMode |= std::ios_base::app;
    }

    if(openMode & CERBERUS_FILE_TRUNCATE)
    {
        m_openMode |= std::ios_base::trunc;
    }
}
//=============================================================================
bool cerberus::data::filesystem::File::isOpen() const
{
    return m_stream.is_open();
}
//=============================================================================
bool cerberus::data::filesystem::File::open()
{
    if(m_stream.is_open())
    {
        throw cerberusIllegalStateExc("File already open");
    }

    if(m_filePath.empty())
    {
        throw cerberusIllegalArgumentExc("Filename is empty");
    }

    if(!File::existsAsFile(m_filePath) && (m_openMode & std::ios_base::out))
    {
        m_stream.open(m_filePath, m_openMode | std::ios_base::trunc);
    }
    else
    {
        m_stream.open(m_filePath, m_openMode);
    }

    if(m_stream.is_open())
    {
        return true;
    }

    return false;
}
//=============================================================================
bool cerberus::data::filesystem::File::close()
{
    if(m_stream.is_open())
    {
        m_stream.close();

        if(m_stream.is_open())
        {
            return false;
        }

        return true;
    }

    throw cerberusIllegalStateExc("File is not open");
}
//=============================================================================
bool cerberus::data::filesystem::File::deleteFromDisk()
{
    if(m_stream.is_open())
    {
        throw cerberusIllegalStateExc("Cannot delete an open file");
    }

    return (std::remove(m_filePath.c_str()) == 0);
}
//=============================================================================
bool cerberus::data::filesystem::File::rename(const std::string& newName)
{
    if(m_stream.is_open())
    {
        throw cerberusIllegalStateExc("Cannot rename an open file");
    }

    return (std::rename(m_filePath.c_str(), newName.c_str()) == 0);
}
//=============================================================================
uint64_t cerberus::data::filesystem::File::size()
{
    if(!m_stream.is_open())
    {
        throw cerberusIllegalStateExc("File is not open");
    }

    std::streampos pos = m_stream.tellg();
    m_stream.seekg(0, m_stream.end);
    uint64_t ret = m_stream.tellg();
    m_stream.seekg(pos);
    return ret;
}
//=============================================================================
bool cerberus::data::filesystem::File::write(const ByteBuffer& bytes)
{
    if(!m_stream.is_open())
    {
        throw cerberusIllegalStateExc("File is not open");
    }

    m_stream.write((const char*)bytes.data(), bytes.size());
    return !(m_stream.fail() || m_stream.bad());
}
//=============================================================================
bool cerberus::data::filesystem::File::writeLine(const std::string& line)
{
    if(!m_stream.is_open())
    {
        throw cerberusIllegalStateExc("File is not open");
    }

    m_stream.write(core::CerberusUtils::strPrint("%s\n", line.c_str()).c_str(), line.length() + 1);
    return !(m_stream.fail() || m_stream.bad());
}
//=============================================================================
void cerberus::data::filesystem::File::read(ByteBuffer& bytes, std::streampos start)
{
    if(!m_stream.is_open())
    {
        throw cerberusIllegalStateExc("File is not open");
    }

    m_stream.seekg(0, std::ios_base::end);
    std::streampos size = m_stream.tellg();

    if(start >= size)
    {
        throw cerberusIllegalArgumentExc("Start parameter is out of bound");
    }

    m_stream.seekg(start);
    std::streamsize bytesToRead = size - start;
    bytes.resize(bytesToRead);
    m_stream.read((char*)bytes.data(), bytesToRead);
}
//=============================================================================
void cerberus::data::filesystem::File::read(ByteBuffer& bytes, std::streampos start, std::streamsize span)
{
    if(!m_stream.is_open())
    {
        throw cerberusIllegalStateExc("File is not open");
    }

    m_stream.seekg(0, std::ios_base::end);
    std::streampos size = m_stream.tellg();

    if(start >= size)
    {
        throw cerberusIllegalArgumentExc("Start parameter is out of bound");
    }

    if((start + span) > size)
    {
        throw cerberusIllegalArgumentExc("Span parameter would exceed file size");
    }

    m_stream.seekg(start);
    bytes.resize(span);
    m_stream.read((char*)bytes.data(), span);
}
//=============================================================================
bool cerberus::data::filesystem::File::readLine(std::string& line)
{
    if(!m_stream.is_open())
    {
        throw cerberusIllegalStateExc("File is not open");
    }

    std::getline(m_stream, line);

    if(m_stream.eof())
    {
        return false;
    }
    else
    {
        return true;
    }
}
//=============================================================================
void cerberus::data::filesystem::File::resetReadCursor()
{
    m_stream.seekg(0);
}
//=============================================================================
void cerberus::data::filesystem::File::resetWriteCursor()
{
    m_stream.seekp(0);
}
//=============================================================================
std::streampos cerberus::data::filesystem::File::readCursor()
{
    return m_stream.tellg();
}
//=============================================================================
std::streampos cerberus::data::filesystem::File::writeCursor()
{
    return m_stream.tellp();
}
//=============================================================================
void cerberus::data::filesystem::File::setReadCursor(std::streampos pos)
{
    m_stream.seekg(pos);
}
//=============================================================================
void cerberus::data::filesystem::File::setWriteCursor(std::streampos pos)
{
    m_stream.seekp(pos);
}
//=============================================================================
void cerberus::data::filesystem::File::moveReadCursor(std::streamoff offset)
{
    m_stream.seekg(offset, std::ios_base::cur);
}
//=============================================================================
void cerberus::data::filesystem::File::moveWriteCursor(std::streamoff offset)
{
    m_stream.seekp(offset, std::ios_base::cur);
}
//=============================================================================

