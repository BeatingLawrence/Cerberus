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
      m_initFlag(false),
      m_mutex()
{
    // noop
}
//=============================================================================
Cerberus& Cerberus::instance()
{
    static Cerberus cerberus;
    return cerberus;
}
//=============================================================================
Cerberus::~Cerberus()
{
    if (instance().m_initFlag)
    {
        deinit();
    }
}
//=============================================================================
void Cerberus::init(const CerberusInitParms& parms)
{
    auto& cerberus = instance();
    mutex::MutexLocker locker(&cerberus.m_mutex);

    if (cerberus.m_initFlag)
    {
        clogWarning("Cerberus already initialized, skipping init() call..");
        return;
    }

    // do the initialization:
    core::CerberusLog::_setup(parms.logSetup);
    cerberus.setLogFileName(parms.logSetup.logFileName);
    cerberus.start();
    // Do other stuff..
    cerberus.m_initFlag = true;
    cdebug("Cerberus init completed");
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
        clogError("Unable to ignore SIGPIPE system signal, using SQL may terminate the process");
    }

#endif
}
//=============================================================================
void Cerberus::init() { init(cerberusDefaultParms()); }
//=============================================================================
void Cerberus::deinit()
{
    auto& cerberus = instance();
    mutex::MutexLocker locker(&cerberus.m_mutex);

    if (cerberus.m_initFlag == false)
    {
        return;
    }

    instance().join(true);
    instance().m_initFlag = false;
    clogInfo("Cerberus Memory Released");
}
//=============================================================================
CerberusInitParms Cerberus::cerberusDefaultParms()
{
    CerberusInitParms toReturn{};
    toReturn.logSetup.disableFormatting   = false;
    toReturn.logSetup.logFileName         = "./last.log";
    toReturn.logSetup.applicationLogLevel = LL_Error;
    toReturn.logSetup.cerberusLogLevel    = LL_Error;

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
void Cerberus::send(message::cerberus_message message) { instance().addMessage(message); }
//=============================================================================
