#include "cerberuscore.h"
#include "../mutex/mutexlocker.h"
#include "../message/slot/stringslot.h"
#include "./cerberuslog.h"
#include "src/cerberusobject.h"
#include "./cerberusfactory.h"
#include "../thread/thread.h"

using namespace cerberus::core;

//=============================================================================
int CerberusCore::tick()
{
    message::cerberus_message message = nextMessage();

    if(!(message->isValid()))
    {
        return 0;
    }

    if(message->id() == CERBERUS_MESSAGE_LOG_ID)
    {
        _writeLineOnFile(message->getSlotAt(0)->to<message::slot::StringSlot>()->value());
        return 0;
    }

    //Process message queue..
    uint32_t destination = message->destinationId();

    if(destination == CERBERUS_INVALID_ID)
    {
        logInfo("Destination of message is invalid, dropping..");
    }
    else
    {
        CerberusObject* found = core::CerberusFactory::_cerberusObjectById(destination);

        if(found == nullptr)
        {
            logInfo("Destination of message is unknown, dropping..");
        }
        else
        {
            if(found->type() == CerberusObject::ObjectType::OT_Thread)
            {
                thread::Thread* foundThread = found->to<thread::Thread>();
                foundThread->addMessage(message);    //ownership transferred
            }
            else
            {
                logInfo("Destination of message cannot accept messages, dropping..");
            }

            //ADD other messages receivers here..
        }
    }

    //Do other stuff..
    return 0;
}
//=============================================================================
void CerberusCore::warmUp()
{
    logInfo("Starting Core Thread..");

    if(!m_logFile.open())
    {
        logWarning("LogFile open failed");
    }
}
//=============================================================================
void CerberusCore::coolDown()
{
    logInfo("Stopping Cerberus Core..");
    _writeLineOnFile("---LOG-END---");
    core::CerberusFactory::_freeMemory();
    logInfo("Closing log file..");
    m_logFile.close();
}
//=============================================================================
void CerberusCore::_writeLineOnFile(const std::string& line)
{
    mutex::MutexLocker locker(&m_fileMutex);
    m_logFile.writeLine(line);
}
//=============================================================================
CerberusCore::CerberusCore() : cerberus::core::CoreThread(), m_logFile(CERBERUS_FILE_WRITE | CERBERUS_FILE_TRUNCATE)
{
}
//=============================================================================
CerberusCore::~CerberusCore()
{
}
//=============================================================================
void CerberusCore::setLogFileName(const std::string& filename)
{
    mutex::MutexLocker locker(&m_fileMutex);

    if(m_logFile.isOpen())
    {
        m_logFile.close();  //Could block
        m_logFile.setFileName(filename);
        m_logFile.open();
    }
    else
    {
        m_logFile.setFileName(filename);
    }
}
//=============================================================================
