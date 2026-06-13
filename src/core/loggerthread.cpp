#include "loggerthread.h"

#include "../cerberus.h"
#include "../data/filesystem/directory.h"

using namespace crb::core;

//=============================================================================
int LoggerThread::tick()
{
    msg_ptr message = next();

    if (message->id() != CRB_MESSAGE_LOG_ID || m_failed.load()) return 0;

    auto str = message->getSlotAt(0)->to<StringSlot>()->value();

    if (m_conf.fileMaxSize)  // log rotation enabled
    {
        LSIZE s = static_cast<LSIZE>(str.size()) + 1;  // add \n char

        if ((m_currentSize + s) > m_conf.fileMaxSize)
        {
            archive();
            if (m_failed.load()) return 0;
        }

        m_currentSize += s;
    }

    if (m_logFile.writeLine(str).fail())
    {
        m_failed.store(true);
        clear();
        m_logFile.close();
        return 0;
    }

    return 0;
}
//=============================================================================
void LoggerThread::warmUp() {}
//=============================================================================
void LoggerThread::coolDown()
{
    if (!m_failed.load())
    {
        tlogInfo("Closing Log file");
        m_logFile.writeLine("---LOG-END---");
        m_logFile.close();
    }

    tlogInfo("Exiting");
}
//=============================================================================
LoggerThread::LoggerThread()
    : crb::Thread(TP_Message),
      m_logFile(),
      m_currentSize(0),
      m_conf()
{
    setThreadName("Logger");
    m_failed.store(false);
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
        tlogError("failed to create log archive directory, disabling log rotation");
        configuration.fileMaxSize = 0;  // disable log rotation
    }
}
//=============================================================================
bool LoggerThread::isFailed() { return m_failed.load(); }
//=============================================================================
void LoggerThread::open()
{
    m_logFile.close();
    m_logFile.path(m_conf.fileName);

    if (m_logFile.open().ok())
    {
        m_failed.store(false);
    }
    else
    {
        tlogError("logfile open failed");
        m_failed.store(true);
        return;
    }

    auto res = m_logFile.size();  // get file size

    if (res.fail("unable to get file size"))
    {
        m_failed.store(true);
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

    m_logFile.move(logDir.append("/").append(archivedFileName)).fail("error while moving log file");

    if (m_conf.logDirMaxSize) checkArchiveSize();

    open();
}
//=============================================================================
void LoggerThread::archiviationName(std::string& fmtStr)
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
void LoggerThread::checkArchiveSize()
{
    Directory dir(m_conf.logDir);
    LSIZE size = 0;

    while (true)
    {
        if (dir.get().fail("failure while getting log archive size")) return;

        size = dir.size();

        if (size > m_conf.logDirMaxSize)
            removeOldestFile();
        else
            break;
    }
}
//=============================================================================
void LoggerThread::removeOldestFile()
{
    Directory dir(m_conf.logDir);
    if (dir.get().fail("failure while getting log archive files")) return;

    auto files = dir.files();
    if (files.empty()) return;

    // first file
    File oldest      = files.front();
    auto oldestBirth = oldest.stat().expect().value.creTime;

    for (auto el = ++files.begin(); el != files.end(); el++)
    {
        auto birth = (*el).stat().expect().value.creTime;
        if (birth.isOlder(oldestBirth))
        {
            oldest      = (*el);
            oldestBirth = oldest.stat().expect().value.creTime;
        }
    }

    oldest.remove();
}
//=============================================================================
