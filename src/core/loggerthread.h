#ifndef CERBERUS_CORE_LOGGERTHREAD_H
#define CERBERUS_CORE_LOGGERTHREAD_H

#include <atomic>

#include "../data/filesystem/file.h"
#include "../thread/thread.h"
#include "../types.h"

namespace crb
{
    namespace core
    {
        class LoggerThread : public Thread
        {
           private:
            File m_logFile;
            LSIZE m_currentSize;

            FileLoggingConf m_conf;

            std::atomic_bool m_failed;

            virtual int tick() override;

            virtual void warmUp() override;

            virtual void coolDown() override;

            void open();

            void archive();

            void archiviationName(std::string& fmtStr);

            void checkArchiveSize();

            void removeOldestFile();

           public:
            LoggerThread();

            void setup(FileLoggingConf configuration);

            bool isFailed();
        };
    }  // namespace core
}  // namespace crb

#endif  // CERBERUS_CORE_LOGGERTHREAD_H
