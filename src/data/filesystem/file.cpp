#include "file.h"
#include <cstdio>
#include "../../exception/exceptioncatalog.h"
#include "../bytebuffer.h"
#include "../../cerberus.h"

#ifdef WINDOWS_SYSTEM
    #include <windows.h>
#else
    #include <unistd.h>
    #include <sys/stat.h>
#endif

//=============================================================================
bool cerberus::data::filesystem::File::exist(const std::string& fileName)
{
    bool exists = false;
#ifdef WINDOWS_SYSTEM

    if(GetFileAttributesA(fileName.c_str()) != INVALID_FILE_ATTRIBUTES)
    {
        exists = true;
    }

#else
    struct stat buffer;
    return (stat(fileName.c_str(), &buffer) == 0);
#endif
    return exists;
}
//=============================================================================
cerberus::data::filesystem::File::File(const std::string& fileName, uint8_t openMode) :
    m_fileName(fileName),
    m_stream(),
    m_openMode(std::ios_base::in)
{
    if(fileName.empty())
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

    m_stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);    //will throw exception if fail or bad
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

    m_stream.open(m_fileName, m_openMode);

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

    return (std::remove(m_fileName.c_str()) == 0);
}
//=============================================================================
bool cerberus::data::filesystem::File::rename(const std::string& newName)
{
    if(m_stream.is_open())
    {
        throw cerberusIllegalStateExc("Cannot rename an open file");
    }

    return (std::rename(m_fileName.c_str(), newName.c_str()) == 0);
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

    m_stream.write(cerberus::Cerberus::strPrint("%s\n", line.c_str()).c_str(), line.length() + 1);
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

