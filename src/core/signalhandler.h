#ifndef CERBERUS_CORE_SIGNALHANDLER_H
#define CERBERUS_CORE_SIGNALHANDLER_H

#include "../thread/thread.h"

namespace crb
{
    namespace core
    {
#ifndef WINDOWS_SYSTEM
        void fillTerminationSignalSet(sigset_t& set);
        void maskTerminationSignalsForCurrentThread();
        const char* signalName(int signo);
#else
        inline void maskTerminationSignalsForCurrentThread() {}
#endif

        class SignalHandler : public crb::Thread
        {
           public:
            SignalHandler();
            ~SignalHandler() override = default;

           private:
            int tick() override;
            void dispatchSignal(int signo);

#ifndef WINDOWS_SYSTEM
            sigset_t m_signalSet{};
#endif
        };
    }  // namespace core
}  // namespace crb

#endif  // CERBERUS_CORE_SIGNALHANDLER_H
