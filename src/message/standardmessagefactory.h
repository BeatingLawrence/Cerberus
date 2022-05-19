#ifndef CERBERUS_MESSAGE_STANDARDMESSAGEFACTORY_H
#define CERBERUS_MESSAGE_STANDARDMESSAGEFACTORY_H

#include "../Cerberus_global.h"
#include "./message.h"

namespace cerberus
{
    namespace message
    {
        class CERBERUS_EXPORT StandardMessageFactory
        {
            private:
                StandardMessageFactory();

            public:
                static cerberus_message createStandardMessage(uint32_t id = CERBERUS_INVALID_ID);
        };
    }
}

#endif // CERBERUS_MESSAGE_STANDARDMESSAGEFACTORY_H
