#include "cerberuscore.h"

#include "../message/slot/stringslot.h"
#include "../mutex/mutexlocker.h"
#include "../thread/thread.h"
#include "./cerberusfactory.h"
#include "./cerberuslog.h"
#include "src/core/cerberusobject.h"

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
        debug("Destination of message is invalid, dropping..");
    }
    else
    {
        CerberusObject* found = core::CerberusFactory::_cerberusObjectById(destination);

        if (found == nullptr)
        {
            debug("Destination of message is unknown, dropping..");
        }
        else
        {
            if (found->type() == CerberusObject::ObjectType::Thread)
            {
                thread::Thread* foundThread = found->to_p<thread::Thread>();
                foundThread->addMessage(message);  // ownership transferred
            }
            else
            {
                debug("Destination of message cannot accept messages, dropping..");
            }

            // ADD other messages receivers here..
        }
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
}
//=============================================================================
void CerberusCore::coolDown()
{
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
    : cerberus::core::CoreThread(),
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
