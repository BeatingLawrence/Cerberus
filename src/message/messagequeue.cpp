#include "messagequeue.h"
#include "../exception/exceptioncatalog.h"

using namespace cerberus::message;

//=============================================================================
cerberus_messageQueue MessageQueue::create()
{
    return cerberus_messageQueue(new MessageQueue());
}
//=============================================================================
cerberus_messageQueue MessageQueue::create(const MessageQueue& other)
{
    return cerberus_messageQueue(new MessageQueue(other));
}
//=============================================================================
MessageQueue::MessageQueue()
{
    // noop
}
//=============================================================================
MessageQueue::MessageQueue(const MessageQueue& other) : m_queue(other.m_queue)
{
    // noop
}
//=============================================================================
void MessageQueue::add(cerberus_message message)
{
    m_queue.push_back(message);
}
//=============================================================================
cerberus_message MessageQueue::nextRemove()
{
    if(m_queue.empty())
    {
        throw cerberusIllegalStateExc("Queue is empty");
    }

    cerberus_message next = m_queue.front();
    m_queue.pop_front();
    return next;
}
//=============================================================================
cerberus_message MessageQueue::next() const
{
    if(m_queue.empty())
    {
        throw cerberusIllegalStateExc("Queue is empty");
    }

    cerberus_message next = m_queue.front();
    return next;
}
//=============================================================================
size_t MessageQueue::size() const
{
    return m_queue.size();
}
//=============================================================================
bool MessageQueue::isEmpty() const
{
    return m_queue.empty();
}
//=============================================================================
