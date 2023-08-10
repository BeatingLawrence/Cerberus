#include "cerberus.h"

#include <openssl/conf.h>
#include <openssl/ssl.h>

#include <cstring>

#include "./mutex/mutexlocker.h"

#ifdef WINDOWS_SYSTEM
#include <windows.h>
#else
#include <signal.h>
#include <unistd.h>
#endif

using namespace cerberus;
using namespace cerberus::core;

//=============================================================================
Cerberus::Cerberus()
    : CerberusCore(),
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
    if (_instance()->m_initFlag)
    {
        deinit();
    }
}
//=============================================================================
void Cerberus::init(const CerberusInitParms& parms)
{
    Cerberus* cerberus = _instance();
    mutex::MutexLocker locker(&cerberus->m_mutex);

    if (cerberus->m_initFlag)
    {
        debug("Cerberus already initialized, skipping init() call..");
        return;
    }

    // do the initialization:
    core::CerberusLog::_setup(parms.logSetup);
    cerberus->setLogFileName(parms.logSetup.logFileName);
    cerberus->start();
    // Do other stuff..
    cerberus->m_initFlag = true;
    debug("Cerberus init completed");
    //
    if (parms.useCiphers)
    {
        SSL_library_init();
        SSL_load_error_strings();
    }
    //
#ifndef WINDOWS_SYSTEM
    // PostgreSQL SIGPIPE Ignoring:
    struct sigaction action = {};
    action.sa_handler       = SIG_IGN;

    if (sigaction(SIGPIPE, &action, nullptr) != 0)
    {
        logError("Unable to ignore SIGPIPE system signal, using SQL may terminate the process");
    }

#endif
}
//=============================================================================
void Cerberus::init() { init(cerberusDefaultParms()); }
//=============================================================================
void Cerberus::deinit()
{
    Cerberus* cerberus = _instance();
    mutex::MutexLocker locker(&cerberus->m_mutex);

    if (cerberus->m_initFlag == false)  // double-check
    {
        return;
    }

    _instance()->join();
    _instance()->m_initFlag = false;
    logInfo("Cerberus Memory Released");
}
//=============================================================================
CerberusInitParms Cerberus::cerberusDefaultParms()
{
    CerberusInitParms toReturn{};
    toReturn.logSetup.disableFormatting = false;
    toReturn.logSetup.logFileName       = "./last.log";
    toReturn.logSetup.logLevel          = LL_Error;
#ifdef WINDOWS_SYSTEM
    toReturn.logSetup.infoRole.foregroundColor    = TERMINAL_FOREGROUND_GREEN;
    toReturn.logSetup.warningRole.foregroundColor = (TERMINAL_FOREGROUND_GREEN | TERMINAL_FOREGROUND_RED);
    toReturn.logSetup.errorRole.foregroundColor   = TERMINAL_FOREGROUND_RED;
    toReturn.logSetup.debugRole.foregroundColor   = (TERMINAL_FOREGROUND_RED | TERMINAL_FOREGROUND_BLUE);
#else
    toReturn.logSetup.infoRole.backgroundColor    = TERMINAL_BACKGROUND_BLACK;
    toReturn.logSetup.warningRole.backgroundColor = TERMINAL_BACKGROUND_BLACK;
    toReturn.logSetup.errorRole.backgroundColor   = TERMINAL_BACKGROUND_BLACK;
    toReturn.logSetup.debugRole.backgroundColor   = TERMINAL_BACKGROUND_BLACK;
    toReturn.logSetup.infoRole.foregroundColor    = TERMINAL_FOREGROUND_GREEN;
    toReturn.logSetup.warningRole.foregroundColor = TERMINAL_FOREGROUND_YELLOW;
    toReturn.logSetup.errorRole.foregroundColor   = TERMINAL_FOREGROUND_RED;
    toReturn.logSetup.debugRole.foregroundColor   = TERMINAL_FOREGROUND_MAGENTA;
#endif
    return toReturn;
}
//=============================================================================
std::string Cerberus::cerberusVersion() { return CERBERUS_VERSION; }
//=============================================================================
void Cerberus::send(message::cerberus_message message) { _instance()->addMessage(message); }
//=============================================================================
