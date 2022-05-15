#ifndef CERBERUS_H
#define CERBERUS_H

#include <string>
#include "Cerberus_global.h"
#include "./register.h"
#include "./mutex/mutex.h"

#define logInfo(text) cerberus::Cerberus::provider()->log(text, cerberus::Cerberus::LogLevel::LL_Info)
#define logWarning(text) cerberus::Cerberus::provider()->log(text, cerberus::Cerberus::LogLevel::LL_Warning)
#define logError(text) cerberus::Cerberus::provider()->log(text, cerberus::Cerberus::LogLevel::LL_Error)
#define debug(text) cerberus::Cerberus::provider()->log(text, cerberus::Cerberus::LogLevel::LL_Debug)

#define thrLogInfo(text) cerberus::Cerberus::provider()->log(text, cerberus::Cerberus::LogLevel::LL_Info, this->name())
#define thrLogWarning(text) cerberus::Cerberus::provider()->log(text, cerberus::Cerberus::LogLevel::LL_Warning, this->name())
#define thrLogError(text) cerberus::Cerberus::provider()->log(text, cerberus::Cerberus::LogLevel::LL_Error, this->name())
#define thrDebug(text) cerberus::Cerberus::provider()->log(text, cerberus::Cerberus::LogLevel::LL_Debug, this->name())

namespace cerberus
{
    struct CERBERUS_EXPORT CerberusCustomizedLogRole
    {
        uint8_t textFormatting[3];  //up to 3 formatting specifiers, 0 will be ignored, see define.h
        uint8_t foregroundColor;    //color specifier
        uint8_t backgroundColor;    //color specifier
    };

    struct CERBERUS_EXPORT CerberusCustomizedTerminal
    {
        CerberusCustomizedLogRole infoRole;
        CerberusCustomizedLogRole warningRole;
        CerberusCustomizedLogRole errorRole;
        CerberusCustomizedLogRole debugRole;
    };

    struct CERBERUS_EXPORT CerberusInitParms
    {
        bool terminalFormattingDisabled;
        CerberusCustomizedTerminal terminal;
    };

    class CERBERUS_EXPORT Cerberus
    {
        private:
            Cerberus();

            bool _isColorSupported();

            bool m_useFormattedTerminal;

            std::string m_infoLogTerminalFormatting;
            std::string m_warningLogTerminalFormatting;
            std::string m_errorLogTerminalFormatting;
            std::string m_debugLogTerminalFormatting;

            static const char* EndOfFormatting;

            std::string _parseFormattingData(const CerberusCustomizedLogRole& data);

            bool m_initFlag;

            Register m_register;

            thread::Thread* m_coreThread;   //has

            static message::slot::cerberus_slot _newSlot(message::slot::BaseSlot::SlotType type);

            static void coreWarmUp();

            static void coreCoolDown();

            static int coreTick(message::cerberus_message message);

            //Thread registering section (for friend classes)

            uint32_t _registerThread(cerberus::thread::Thread* thread, const std::string& name = std::string());

            uint32_t _registerCerberusObject(CerberusObject* object);

        public:
            friend class ::cerberus::CerberusObject;

            enum LogLevel
            {
                LL_Info,
                LL_Warning,
                LL_Error,
                LL_Debug,
            };

            static Cerberus* provider();

            ~Cerberus();

            //Performs the init sequence of the Cerberus framework. This operation must precede any others
            void init(const CerberusInitParms& parms);

            static std::string strPrint(const char* format, ...);

            static CerberusInitParms cerberusDefaultParms();

            //Logging section:                                      ===================================================

            //Logs the given string to stdout/stderr according to the specified logLevel
            static void log(const std::string& str, LogLevel logLevel = LL_Info, const std::string& author = std::string());

            //Message factory section:                              ===================================================

            //Adds a template of the given message to the register, returning the chosen typeID
            uint32_t registerMessage(const message::Message& message, const std::string& name = std::string());

            //Removes a message template with given ID from the register
            void forgetMessage(uint32_t typeID);

            //Returns a registered message ID searched by its name
            uint32_t messageTypeIdByName(const std::string& name) const;

            //Factory of messages. A call to this method will return an empty but structured message.
            //Will throw an exception if typeID was not found.
            message::cerberus_message messageConstruct(uint32_t typeID) const;


    };
}

#endif // CERBERUS_H
