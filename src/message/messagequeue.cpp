#include "messagequeue.h"

using namespace cerberus::message;

//=============================================================================
MessageQueue::MessageQueue()
{
    // noop
}
//=============================================================================
void MessageQueue::add(cerberus_message message) { m_queue.push_back(message); }
//=============================================================================
cerberus_message MessageQueue::next()
{
    if (m_queue.empty())
    {
        return Message::create();
    }

    cerberus_message next = m_queue.front();
    m_queue.pop_front();
    return next;
}
//=============================================================================
cerberus_message MessageQueue::nextKeep() const
{
    if (m_queue.empty())
    {
        return Message::create();
    }

    cerberus_message next = m_queue.front();
    return next;
}
//=============================================================================
size_t MessageQueue::size() const { return m_queue.size(); }
//=============================================================================
bool MessageQueue::isEmpty() const { return m_queue.empty(); }
//=============================================================================
void MessageQueue::clear() { m_queue.clear(); }
//=============================================================================
