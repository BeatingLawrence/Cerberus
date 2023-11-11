#include "cerberuscore.h"

#include "../message/slot/stringslot.h"
#include "../mutex/mutexlocker.h"
#include "../thread/thread.h"
#include "./cerberuslog.h"
#include "./cerberusregister.h"

using namespace cerberus::core;

//=============================================================================
int CerberusCore::tick()
{
    message::cerberus_message message = nextMessage();

    if (!(message->isValid()))
    {
        return 0;
    }

    if (message->id() == CERBERUS_MESSAGE_LOG_ID)
    {
        _writeLineOnFile(message->getSlotAt(0)->to<message::slot::StringSlot>()->value());
        return 0;
    }

    // Process message queue..
    uint32_t destination = message->destinationId();

    if (destination == CERBERUS_INVALID_ID)
    {
        debug("Destination of message is invalid, dropping");
    }
    else
    {
        CerberusRegister::sendMsgToObj(destination, message);
    }

    // Do other stuff..
    return 0;
}
//=============================================================================
void CerberusCore::warmUp()
{
    debug("Starting Core Thread..");

    if (!m_logFile.open() && !m_logFile.fileName().empty())
    {
        debug("LogFile open failed");
    }

    debug("Starting FastLoop");
    m_fastLoop.start();
}
//=============================================================================
void CerberusCore::coolDown()
{
    debug("Stopping FastLoop");
    m_fastLoop.terminate();
    debug("Stopping Cerberus Core..");
    _writeLineOnFile("---LOG-END---");
    debug("Closing log file..");
    m_logFile.close();
}
//=============================================================================
void CerberusCore::_writeLineOnFile(const std::string& line)
{
    mutex::MutexLocker locker(&m_fileMutex);

    if (m_logFile.isOpen())
    {
        m_logFile.writeLine(line);
    }
}
//=============================================================================
CerberusCore::CerberusCore()
    : cerberus::thread::Thread(TP_NonPeriodic, time::Time(), "Cerberus-core"),
      m_logFile(FOM_ReadWriteAppend)
{
}
//=============================================================================
CerberusCore::~CerberusCore() {}
//=============================================================================
void CerberusCore::setLogFileName(const std::string& filename)
{
    mutex::MutexLocker locker(&m_fileMutex);

    if (m_logFile.isOpen())
    {
        m_logFile.close();  // Could block
        m_logFile.setFileName(filename);

        if (!filename.empty())
        {
            m_logFile.open();
        }
    }
    else
    {
        m_logFile.setFileName(filename);
    }
}
//=============================================================================
