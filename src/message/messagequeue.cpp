#include "messagequeue.h"

#include "./message.h"

using namespace cerberus;

//=============================================================================
MessageQueue::MessageQueue()
{
    // noop
}
//=============================================================================
MessageQueue::MessageQueue(MessageQueue &&other)
    : m_queue(std::move(other.m_queue))
{
}
//=============================================================================
void MessageQueue::add(cerberus_message message) { m_queue.push_back(message); }
//=============================================================================
cerberus_message MessageQueue::next()
{
    if (m_queue.empty()) return Message::create();

    cerberus_message next = m_queue.front();  // increment ref counter
    m_queue.pop_front();
    return std::move(next);
}
//=============================================================================
cerberus_message MessageQueue::nextKeep() const
{
    if (m_queue.empty()) return Message::create();

    return m_queue.front();  // copy constructor will deep-copy
}
//=============================================================================
size_t MessageQueue::size() const { return m_queue.size(); }
//=============================================================================
bool MessageQueue::isEmpty() const { return m_queue.empty(); }
//=============================================================================
void MessageQueue::clear() { m_queue.clear(); }
//=============================================================================
