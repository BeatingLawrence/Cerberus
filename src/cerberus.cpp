#include "cerberus.h"
#include "./mutex/mutexlocker.h"
#include "./core/cerberuslog.h"
#include <cstring>

#ifdef WINDOWS_SYSTEM
    #include <windows.h>
#else
    #include <unistd.h>
#endif

using namespace cerberus;
using namespace cerberus::core;

//=============================================================================
Cerberus::Cerberus() : CerberusCore(),
    m_initFlag(false)
{
    // noop
}
//=============================================================================
Cerberus* Cerberus::_instance()
{
    static Cerberus cerberus;
    return &cerberus;
}
//=============================================================================
Cerberus::~Cerberus()
{
    if(_instance()->m_initFlag)
    {
        deinit();
    }
}
//=============================================================================
CerberusObject* Cerberus::cerberusObjectById(uint32_t id)
{
    return m_factory.cerberusObjectById(id);
}
//=============================================================================
void Cerberus::freeRegisterMemory()
{
    m_factory.freeMemory();
}
//=============================================================================
uint32_t Cerberus::_registerCerberusObject(CerberusObject* object)
{
    return _instance()->m_factory.registerCerberusObject(object);
}
//=============================================================================
void Cerberus::_unregisterCerberusObject(uint32_t id)
{
    _instance()->m_factory.unregisterCerberusObject(id);
}
//=============================================================================
CerberusLog* Cerberus::_logInstance()
{
    return &(_instance()->m_log);
}
//=============================================================================
void Cerberus::init(const CerberusInitParms* parms)
{
    Cerberus* cerberus = _instance();
    mutex::MutexLocker locker(&cerberus->m_mutex);

    if(cerberus->m_initFlag)
    {
        logInfo("Cerberus already initted, skipping init() call..");
        return;
    }

    // do the initialization:
    cerberus->m_log.setup(parms->logSetup);
    cerberus->setLogFileName(parms->logSetup.logFileName);
    cerberus->start();
    //Do other stuff..
    cerberus->m_initFlag = true;
    logInfo("Cerberus init completed");
}
//=============================================================================
void Cerberus::deinit()
{
    Cerberus* cerberus = _instance();
    mutex::MutexLocker locker(&cerberus->m_mutex);

    if(cerberus->m_initFlag == false)   //double-check
    {
        return;
    }

    _instance()->join();
    _instance()->m_initFlag = false;
    logInfo("Cerberus Memory Released");
}
//=============================================================================
CerberusInitParms* Cerberus::cerberusDefaultParms()
{
    CerberusInitParms* toReturn = new CerberusInitParms;
    memset(toReturn, 0, sizeof(CerberusInitParms));
    toReturn->logSetup.disableFormatting = false;
    toReturn->logSetup.logFileName = (char*)malloc(13);
    memcpy(toReturn->logSetup.logFileName, "./latest.log", 13);
#ifdef WINDOWS_SYSTEM
    toReturn->terminal.infoRole.foregroundColor = TERMINAL_FOREGROUND_GREEN;
    toReturn->terminal.warningRole.foregroundColor = (TERMINAL_FOREGROUND_GREEN | TERMINAL_FOREGROUND_RED);
    toReturn->terminal.errorRole.foregroundColor = TERMINAL_FOREGROUND_RED;
    toReturn->terminal.debugRole.foregroundColor = (TERMINAL_FOREGROUND_RED | TERMINAL_FOREGROUND_BLUE);
#else
    toReturn->logSetup.infoRole.backgroundColor = TERMINAL_BACKGROUND_BLACK;
    toReturn->logSetup.warningRole.backgroundColor = TERMINAL_BACKGROUND_BLACK;
    toReturn->logSetup.errorRole.backgroundColor = TERMINAL_BACKGROUND_BLACK;
    toReturn->logSetup.debugRole.backgroundColor = TERMINAL_BACKGROUND_BLACK;
    toReturn->logSetup.infoRole.foregroundColor = TERMINAL_FOREGROUND_GREEN;
    toReturn->logSetup.warningRole.foregroundColor = TERMINAL_FOREGROUND_YELLOW;
    toReturn->logSetup.errorRole.foregroundColor = TERMINAL_FOREGROUND_RED;
    toReturn->logSetup.debugRole.foregroundColor = TERMINAL_FOREGROUND_MAGENTA;
#endif
    return toReturn;
}
//=============================================================================
uint32_t Cerberus::registerMessage(const message::Message& message, const std::string& name)
{
    return _instance()->m_factory.registerMessage(message, name);
}
//=============================================================================
uint32_t Cerberus::messageIdByName(const std::string& name)
{
    return _instance()->m_factory.messageIdByName(name);
}
//=============================================================================
message::cerberus_message Cerberus::messageConstruct(uint32_t id)
{
    return _instance()->m_factory.messageConstruct(id);
}
//=============================================================================
void Cerberus::send(message::cerberus_message message)
{
    _instance()->addMessage(message);
}
//=============================================================================
uint32_t Cerberus::threadIdByName(const std::string& name)
{
    return _instance()->threadIdByName(name);
}
//=============================================================================
