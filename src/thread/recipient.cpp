#include "recipient.h"

#include "../cerberus.h"
#include "../core/cerberuslog.h"
#include "../message/message.h"  // IWYU pragma: export
#include "../thread/mutexlocker.h"

using namespace cerberus;

//=============================================================================
void Recipient::_check() const
{
    if (!m_queueWarningBytes) return;

    if (m_queueBytes > m_queueWarningBytes)
    {
        logWarning("Queue size limit reached %u/%u", m_queueBytes, m_queueWarningBytes);
    }
}
//=============================================================================
msg_ptr Recipient::next()
{
    MutexLocker loc(m_mutex);
    if (m_queue.empty()) return msg_ptr();  // return unconsistent ptr

    msg_ptr next = std::move(m_queue.front());
    m_queue.pop_front();
    m_queueBytes -= next.memFootprint();  // performance counter
    return std::move(next);
}
//=============================================================================
msg_ptr Recipient::nextKeep() const
{
    MutexLocker loc(m_mutex);
    if (m_queue.empty()) return msg_ptr();  // return unconsistent ptr

    return m_queue.front().duplicate();  // deep-copy
}
//=============================================================================
void Recipient::clear()
{
    MutexLocker loc(m_mutex);
    m_queue.clear();
}
//=============================================================================
void Recipient::setQueueWarning(SIZE bytes)
{
    MutexLocker loc(m_mutex);
    m_queueWarningBytes = bytes;
}
//=============================================================================
SIZE Recipient::getQueueBytesCount()
{
    MutexLocker loc(m_mutex);
    return m_queueBytes;
}
//=============================================================================
void Recipient::newMsg()
{
    // nop
}
//=============================================================================
void Recipient::newMsg_first()
{
    // nop
}
//=============================================================================
Recipient::Recipient()
    : m_queueBytes(0),
      m_queueWarningBytes(0)
{
}
//=============================================================================
void Recipient::addMessage(msg_ptr message)
{
    MutexLocker loc(m_mutex);
    m_queue.push_back(std::move(message));

    m_queueBytes += message.memFootprint();  // performance counter
    _check();

    // signal a new message
    if (m_queue.size() == 1) newMsg_first();
    newMsg();
}
//=============================================================================
size_t Recipient::size() const
{
    MutexLocker loc(m_mutex);
    return m_queue.size();
}
//=============================================================================
bool Recipient::hasMessage() const { return size(); }
//=============================================================================
