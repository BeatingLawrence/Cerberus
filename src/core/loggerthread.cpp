#include "loggerthread.h"

#include "../cerberus.h"
#include "src/message/slot/stringslot.h"

using namespace cerberus::core;

//=============================================================================
int LoggerThread::tick()
{
    cerberus_message message = nextMessage();

    if (message->id() != CERBERUS_MESSAGE_LOG_ID || m_failed.test()) return 0;

    if (m_logFile.writeLine(message->getSlotAt(0)->to<message::slot::StringSlot>()->value()).fail())
    {
        m_failed.test_and_set();
        discardMessageQueue();
        m_logFile.close();
    }

    return 0;
}
//=============================================================================
void LoggerThread::warmUp() {}
//=============================================================================
void LoggerThread::coolDown()
{
    if (!m_failed.test())
    {
        logInfo("Closing Log file");
        m_logFile.writeLine("---LOG-END---");
        m_logFile.close();
    }

    logInfo("Stopping Logger Thread");
}
//=============================================================================
LoggerThread::LoggerThread()
    : cerberus::thread::Thread(TP_NonPeriodic, "Logger")
{
    m_failed.clear();
    m_logFile.setOpenMode(FOM_ReadWriteAppend);
}
//=============================================================================
void LoggerThread::setup(const std::string &filename, SIZE fileMaxSize)
{
    m_logFile.close();
    m_logFile.setFileName(filename);
    m_logFileMaxSize = fileMaxSize;
    open();
}
//=============================================================================
bool LoggerThread::isFailed() { return m_failed.test(); }
//=============================================================================
void LoggerThread::open()
{
    if (m_logFile.open().ok())
    {
        logInfo("LogFile ready");
        m_failed.clear();
    }
    else
    {
        logError("LogFile open failed");
        m_failed.test_and_set();
    }
}
//=============================================================================
