#ifndef CERBERUS_CORE_CERBERUSCORE_H
#define CERBERUS_CORE_CERBERUSCORE_H

/*  This is the CerberusCore class.
 *
 *  The CerberusCore is the main thread of execution of the framework.
 *  It's used to route messages, log on file, create other Threads and other stuff..
 *
 */

#include "../data/filesystem/file.h"
#include "src/core/fastloop.h"
#include "src/thread/thread.h"

namespace cerberus
{
    namespace core
    {
        class CerberusCore : protected cerberus::thread::Thread
        {
           private:
            virtual int tick() override;

            virtual void warmUp() override;

            virtual void coolDown() override;

            data::filesystem::File m_logFile;

            mutex::Mutex m_fileMutex;

            void _writeLineOnFile(const std::string& line);

           public:
            FastLoop m_fastLoop;

            CerberusCore();

            virtual ~CerberusCore();

            void setLogFileName(const std::string& filename);
        };
    }  // namespace core
}  // namespace cerberus

#endif  // CERBERUS_CORE_CERBERUSCORE_H
