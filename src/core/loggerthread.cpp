#include "loggerthread.h"

#include "../cerberus.h"

using namespace cerberus::core;

//=============================================================================
int LoggerThread::tick()
{
    cerberus_message message = nextMessage();

    if (message->id() != CERBERUS_MESSAGE_LOG_ID || m_failed.test()) return 0;

    if (m_logFile.writeLine(message->getSlotAt(0)->to<StringSlot>()->value()).fail())
    {
        m_failed.test_and_set();
        discardMessageQueue();
        m_logFile.close();
        return 0;
    }

    if (!m_conf.fileMaxSize) return 0;  // rotation disabled

    // increment file size
    m_currentSize += message->getConstSlotAt(0)->to<StringSlot>()->value().size();

    if (m_currentSize > m_conf.fileMaxSize)
    {
        archive();
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
    : cerberus::Thread(TP_Message, "Logger"),
      m_logFile(),
      m_currentSize(0),
      m_conf()
{
    m_failed.clear();
    m_logFile.setOpenMode(FOM_ReadWriteAppend);
}
//=============================================================================
void LoggerThread::setup(FileLoggingConf configuration)
{
    m_conf = configuration;
    open();

    if (!configuration.fileMaxSize) return;

    // check archive directory presence
    if (File::existsAsDirectory(configuration.logDir).ok()) return;  // ok

    if (File::createDirectory(configuration.logDir).fail())
    {
        logError("failed to create log archive directory");
        configuration.fileMaxSize = 0;  // disable log rotation
    }
}
//=============================================================================
bool LoggerThread::isFailed() { return m_failed.test(); }
//=============================================================================
void LoggerThread::open()
{
    m_logFile.close();
    m_logFile.setFileName(m_conf.fileName);

    if (m_logFile.open().ok())
    {
        logInfo("LogFile ready");
        m_failed.clear();
    }
    else
    {
        logError("LogFile open failed");
        m_failed.test_and_set();
        return;
    }

    auto res = m_logFile.size();  // get file size

    if (res.fail(true))
    {
        m_failed.test_and_set();
        return;
    }

    m_currentSize = res.value;
}
//=============================================================================
void LoggerThread::archive()
{
    m_logFile.close();

    std::string archivedFileName = m_conf.fileNameFmt;
    archiviationName(archivedFileName);

    std::string logDir = m_conf.logDir;

    m_logFile.move(logDir.append("/").append(archivedFileName)).ok(true);

    open();
}
//=============================================================================
void LoggerThread::archiviationName(std::string &fmtStr)
{
    auto current = DateTime::current();

    std::string tmp;
    tmp = CerberusUtils::strPrint_uint(current.days());
    CerberusUtils::replaceAll(fmtStr, "%D", tmp);

    tmp = CerberusUtils::strPrint_uint(current.months());
    CerberusUtils::replaceAll(fmtStr, "%M", tmp);

    tmp = CerberusUtils::strPrint_uint(current.years());
    CerberusUtils::replaceAll(fmtStr, "%Y", tmp);

    tmp = CerberusUtils::strPrint_uint(current.hours());
    CerberusUtils::replaceAll(fmtStr, "%h", tmp);

    tmp = CerberusUtils::strPrint_uint(current.minutes());
    CerberusUtils::replaceAll(fmtStr, "%m", tmp);

    tmp = CerberusUtils::strPrint_uint(current.seconds());
    CerberusUtils::replaceAll(fmtStr, "%s", tmp);
}
//=============================================================================
