#include "signalhandler.h"

#ifndef WINDOWS_SYSTEM
#include <cerrno>
#include <signal.h>
#include <sys/wait.h>
#endif

#include "../cerberus.h"
#include "../log.h"
#include "../message/message.h"

using namespace crb;
using namespace crb::core;

#ifndef WINDOWS_SYSTEM
//=============================================================================
void crb::core::fillTerminationSignalSet(sigset_t& set)
{
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGHUP);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);
    sigaddset(&set, SIGCHLD);
}
//=============================================================================
void crb::core::maskTerminationSignalsForCurrentThread()
{
    sigset_t set;
    fillTerminationSignalSet(set);
    pthread_sigmask(SIG_BLOCK, &set, nullptr);
}
//=============================================================================
const char* crb::core::signalName(int signo)
{
    switch (signo)
    {
        case SIGINT:
            return "SIGINT";
        case SIGTERM:
            return "SIGTERM";
        case SIGHUP:
            return "SIGHUP";
        case SIGQUIT:
            return "SIGQUIT";
        case SIGUSR1:
            return "SIGUSR1";
        case SIGUSR2:
            return "SIGUSR2";
        case SIGCHLD:
            return "SIGCHLD";
        default:
            return "UNKNOWN";
    }
}
#endif

//=============================================================================
SignalHandler::SignalHandler()
    : crb::Thread(crb::TP_Trigger)
{
    setThreadName("signalhandler");

#ifndef WINDOWS_SYSTEM
    fillTerminationSignalSet(m_signalSet);
#endif
}
//=============================================================================
int SignalHandler::tick()
{
#ifdef WINDOWS_SYSTEM
    crb::Thread::sleep(crb::TimeFrame(100, crb::TimeFrame::U_MilliSecond));
    return 0;
#else
    int signo = 0;
    const timespec waitTimeout{0, 100000000};  // Bound shutdown latency while waiting for signals.
    const int ret = sigtimedwait(&m_signalSet, nullptr, &waitTimeout);
    if (ret < 0)
    {
        if (errno == EAGAIN || errno == EINTR)
        {
            reschedule();
            return 0;
        }

        tlogWarning("SignalHandler: sigtimedwait failed: %d", errno);
        crb::Thread::sleep(crb::TimeFrame(10, crb::TimeFrame::U_MilliSecond));
        reschedule();
        return 0;
    }

    signo = ret;
    tlogInfo("SignalHandler: received %s (%d)", signalName(signo), signo);
    dispatchSignal(signo);
    return 0;
#endif
}
//=============================================================================
void SignalHandler::dispatchSignal(int signo)
{
#ifdef WINDOWS_SYSTEM
    (void)signo;
#else
    switch (signo)
    {
        case SIGCHLD:
        {
            int status = 0;
            pid_t pid = 0;

            while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
            {
                tlogInfo("SignalHandler: reaped child pid=%ld status=%d",
                         (long)pid,
                         status);
            }

            reschedule();
            break;
        }

        case SIGINT:
        case SIGTERM:
        case SIGHUP:
        case SIGQUIT:
        case SIGUSR1:
        case SIGUSR2:
        {
            auto msg = crb::Message::create(CRB_MESSAGE_TERM_ID);
            crb::OpRes r = broadcast(msg, 0);
            if (r != crb::OR_OK && r != crb::OR_NotFound)
                tlogWarning("SignalHandler: termination dispatch failed: %s", r.toStr().c_str());
            break;
        }

        default:
            break;
    }
#endif
}
