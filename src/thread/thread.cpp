#include "thread.h"
#include <chrono>

#include "../mutex/mutexlocker.h"

//=============================================================================
void cerberus::thread::Thread::_staticThread(Thread* context)
{
    context->_thread();
}
//=============================================================================
void cerberus::thread::Thread::_thread()
{
    while(!_getTerminateFlag())
    {
        if(_getExecuteFlag())
        {
            m_retValue = tick();
        }
        else
        {
            std::this_thread::yield();
        }

        if(m_periodic)
        {
            std::this_thread::sleep_for(m_period);
        }
    }
}
//=============================================================================
void cerberus::thread::Thread::_setExecuteFlag(bool state)
{
    mutex::MutexLocker locker(&m_mutex);
    m_executeFlag = state;
}
//=============================================================================
void cerberus::thread::Thread::_setTerminateFlag(bool state)
{
    mutex::MutexLocker locker(&m_mutex);
    m_terminateFlag = state;
}
//=============================================================================
bool cerberus::thread::Thread::_getExecuteFlag()
{
    mutex::MutexLocker locker(&m_mutex);
    return m_executeFlag;
}
//=============================================================================
bool cerberus::thread::Thread::_getTerminateFlag()
{
    mutex::MutexLocker locker(&m_mutex);
    return m_terminateFlag;
}
//=============================================================================
int cerberus::thread::Thread::tick()
{
    std::this_thread::yield();
    return 0;
}
//=============================================================================
cerberus::thread::Thread::Thread(const time::Time& time) :
    m_thread(_staticThread, this),
    m_periodic(false),
    m_executeFlag(false),
    m_terminateFlag(false),
    m_retValue(0)
{
    if(time.isValid())
    {
        m_period = std::chrono::microseconds(time.getMicroseconds());
        m_periodic = true;
    }
}
//=============================================================================
cerberus::thread::Thread::~Thread()
{
    if(_getTerminateFlag() == false)
    {
        _setTerminateFlag(true);
        join();
    }
}
//=============================================================================
void cerberus::thread::Thread::start()
{
    _setExecuteFlag(true);
}
//=============================================================================
void cerberus::thread::Thread::stop()
{
    _setExecuteFlag(false);
}
//=============================================================================
int cerberus::thread::Thread::join()
{
    if(!m_thread.joinable())
    {
        return 0;
    }

    m_thread.join();
    return m_retValue;
}
//=============================================================================
void cerberus::thread::Thread::terminate()
{
    _setTerminateFlag(true);
}
//=============================================================================
