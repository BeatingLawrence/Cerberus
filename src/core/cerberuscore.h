#ifndef CERBERUS_CORE_CERBERUSCORE_H
#define CERBERUS_CORE_CERBERUSCORE_H

#include "./corethread.h"
#include "../data/filesystem/file.h"

namespace cerberus
{
    class CerberusObject;

    namespace core
    {
        class CerberusCore : public cerberus::core::CoreThread
        {
            private:
                virtual int tick() override;

                virtual void warmUp() override;

                virtual void coolDown() override;

                data::filesystem::File m_logFile;

                mutex::Mutex m_fileMutex;

                void _writeLineOnFile(const std::string& line);

            public:
                CerberusCore();

                ~CerberusCore();

                void setLogFileName(const std::string& filename);
        };
    }
}

#endif // CERBERUS_CORE_CERBERUSCORE_H
