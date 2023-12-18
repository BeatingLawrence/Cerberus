#include "loggerthread.h"

#include "../cerberus.h"
#include "src/message/slot/stringslot.h"

using namespace cerberus::core;

//=============================================================================
int LoggerThread::tick()
{
    message::cerberus_message message = nextMessage();

    if (message->id() != CERBERUS_MESSAGE_LOG_ID || m_failed.test()) return 0;

    if (m_logFile.writeLine(message->getSlotAt(0)->to<message::slot::StringSlot>()->value()).fail())
    {
        m_failed.test_and_set();
        discardMessageQueue();
    }

    return 0;
}
//=============================================================================
void LoggerThread::warmUp() {}
//=============================================================================
void LoggerThread::coolDown()
{
    m_logFile.writeLine("---LOG-END---");
    m_logFile.close();
    m_failed.test_and_set();  // prevent logger file usage
    logInfo("Closing log file");
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
        logInfo("LogFile open succeeded");
        m_failed.clear();
    }
    else
    {
        logError("LogFile open failed");
        m_failed.test_and_set();
    }
}
//=============================================================================
