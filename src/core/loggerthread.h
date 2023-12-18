#ifndef CERBERUS_CORE_LOGGERTHREAD_H
#define CERBERUS_CORE_LOGGERTHREAD_H

#include <atomic>

#include "../data/filesystem/file.h"
#include "../thread/thread.h"

namespace cerberus
{
    namespace core
    {
        class LoggerThread : public cerberus::thread::Thread
        {
           private:
            data::filesystem::File m_logFile;
            SIZE m_logFileMaxSize;
            std::atomic_flag m_failed;

            virtual int tick() override;

            virtual void warmUp() override;

            virtual void coolDown() override;

           public:
            LoggerThread();

            void setup(const std::string& filename, SIZE fileMaxSize);

            bool isFailed();

            void open();
        };

    }  // namespace core
}  // namespace cerberus

#endif  // CERBERUS_CORE_LOGGERTHREAD_H
