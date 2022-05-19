#include "mutex.h"

using namespace cerberus::mutex;

//============================================================================
Mutex::Mutex() : m_mutex()
{
    // noop
}
//============================================================================
bool Mutex::lock(bool block)
{
    if(block)
    {
        m_mutex.lock();
        return true;
    }
    else
    {
        return m_mutex.try_lock();
    }
}
//=============================================================================
void Mutex::unlock()
{
    m_mutex.unlock();
}
//=============================================================================
