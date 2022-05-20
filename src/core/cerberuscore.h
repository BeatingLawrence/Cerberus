#ifndef CERBERUS_CORE_CERBERUSCORE_H
#define CERBERUS_CORE_CERBERUSCORE_H

#include "../thread/thread.h"
#include "../data/filesystem/file.h"

namespace cerberus
{
    namespace core
    {
        class CerberusCore : public cerberus::thread::Thread
        {
            private:
                virtual int tick() override;

                virtual void warmUp() override;

                virtual void coolDown() override;

                data::filesystem::File m_logFile;

                mutex::Mutex m_fileMutex;

            protected:
                virtual CerberusObject* cerberusObjectById(uint32_t id) = 0;

                virtual void freeRegisterMemory() = 0;

            public:
                CerberusCore();

                ~CerberusCore();

                void setLogFileName(const std::string& filename);
        };
    }
}

#endif // CERBERUS_CORE_CERBERUSCORE_H
