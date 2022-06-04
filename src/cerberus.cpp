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
    core::CerberusLog::_setup(parms->logSetup);
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
    toReturn->logSetup.infoRole.foregroundColor = TERMINAL_FOREGROUND_GREEN;
    toReturn->logSetup.warningRole.foregroundColor = (TERMINAL_FOREGROUND_GREEN | TERMINAL_FOREGROUND_RED);
    toReturn->logSetup.errorRole.foregroundColor = TERMINAL_FOREGROUND_RED;
    toReturn->logSetup.debugRole.foregroundColor = (TERMINAL_FOREGROUND_RED | TERMINAL_FOREGROUND_BLUE);
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
void Cerberus::send(message::cerberus_message message)
{
    _instance()->addMessage(message);
}
//=============================================================================
