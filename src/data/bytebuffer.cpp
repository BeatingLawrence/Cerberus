#include "bytebuffer.h"

#include <string>

using namespace cerberus::data;

//=========================================================
ByteBuffer::ByteBuffer(size_t size):
    m_bytes(new std::vector<unsigned char>(size, 0x00))
{
    // noop
}
//=========================================================
ByteBuffer::ByteBuffer():
    m_bytes(new std::vector<unsigned char>())
{
}
//=========================================================
ByteBuffer::ByteBuffer(const ByteBuffer& other):
    m_bytes(new std::vector<unsigned char>(*(other.m_bytes)))
{
    // noop
}
//=========================================================
ByteBuffer::ByteBuffer(ByteBuffer&& other):
    m_bytes(other.m_bytes)
{
    other.m_bytes = nullptr;
}
//=========================================================
ByteBuffer::ByteBuffer(const char* chars):
    m_bytes(new std::vector<unsigned char>(1, 0x00))
{
    unsigned int counter = 0;

    while(*chars != '\0')
    {
        chars++;
        counter++;
    }

    chars--;
    m_bytes->resize(counter);

    while(counter != 0)
    {
        m_bytes->at(counter - 1) = *chars;
        chars--;
        counter--;
    }
}
//=========================================================
ByteBuffer::~ByteBuffer()
{
    if(m_bytes != nullptr)
    {
        delete m_bytes;
    }
}
//=========================================================
unsigned char* ByteBuffer::data()
{
    return m_bytes->data();
}
//=========================================================
const unsigned char* ByteBuffer::data() const
{
    return m_bytes->data();
}
//=========================================================
unsigned char& ByteBuffer::operator [](size_t index)
{
    return m_bytes->at(index);
}
//=========================================================
ByteBuffer ByteBuffer::subBuffer(size_t pos, size_t len)
{
    ByteBuffer toReturn(len);

    for(size_t i = 0; i < len; i++)
    {
        toReturn[i] = m_bytes->at(pos + i);
    }

    return toReturn;
}
//=========================================================
bool ByteBuffer::operator ==(const ByteBuffer& other)
{
    if(m_bytes->size() != other.m_bytes->size())
    {
        return false;
    }

    size_t c = 0;

    for(size_t i = 0; i < m_bytes->size(); i++)
    {
        if(m_bytes->at(i) != other[c++])
        {
            return false;
        }
    }

    return true;
}
//=========================================================
void ByteBuffer::operator +=(const ByteBuffer& other)
{
    size_t next = m_bytes->size();
    m_bytes->resize(m_bytes->size() + other.size());

    for(auto& el : * (other.m_bytes))
    {
        m_bytes->at(next++) = el;
    }
}
//=========================================================
void ByteBuffer::appendString(const char* chars)
{
    std::string str(chars);
    size_t next = m_bytes->size();
    m_bytes->resize(m_bytes->size() + str.length());

    for(auto& el : str)
    {
        m_bytes->at(next++) = el;
    }
}
//=========================================================
void ByteBuffer::operator +=(unsigned char c)
{
    m_bytes->resize(m_bytes->size() + 1);
    m_bytes->at(m_bytes->size() - 1) = c;
}
//=========================================================
const unsigned char& ByteBuffer::operator [](size_t index) const
{
    return m_bytes->at(index);
}
//=========================================================
size_t ByteBuffer::size() const
{
    return m_bytes->size();
}
//=========================================================
void ByteBuffer::resize(size_t size)
{
    m_bytes->resize(size);
}
//=========================================================
