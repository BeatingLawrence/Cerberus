#include "corethread.h"
#include <chrono>

//=============================================================================
void cerberus::core::CoreThread::_staticThread(CoreThread* context)
{
    context->_thread();
}
//=============================================================================
void cerberus::core::CoreThread::_thread()
{
    bool firstRun = true;

    while(!getTerminateFlag())
    {
        if(getPausedFlag())     // paused
        {
            std::this_thread::yield();
        }
        else                    // execute
        {
            if(firstRun)
            {
                warmUp();
                firstRun = false;
            }

            if(isQueueEmpty())
            {
                std::this_thread::yield();
            }
            else
            {
                m_retValue = tick();
            }
        }
    }

    coolDown();
}
//=============================================================================
cerberus::core::CoreThread::CoreThread() :
    ThreadBase(),
    m_thread(_staticThread, this),
    m_retValue(0)
{
    // noop
}
//=============================================================================
cerberus::core::CoreThread::CoreThread::~CoreThread()
{
    join(true);
}
//=============================================================================
void cerberus::core::CoreThread::start()
{
    setPausedFlag(false);
}
//=============================================================================
void cerberus::core::CoreThread::stop()
{
    setPausedFlag(true);
}
//=============================================================================
int cerberus::core::CoreThread::join(bool stop)
{
    if(stop)
    {
        terminate();
    }

    if(m_thread.joinable())
    {
        m_thread.join();
    }

    return m_retValue;
}
//=============================================================================
void cerberus::core::CoreThread::terminate()
{
    setTerminateFlag(true);
}
//=============================================================================
