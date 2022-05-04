#ifndef CERBERUS_H
#define CERBERUS_H

#include <string>
#include "Cerberus_global.h"
#include "./register.h"
#include "./mutex/mutex.h"

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
        public:

            enum LogLevel
            {
                LL_Info,
                LL_Warning,
                LL_Error,
                LL_Debug,
            };

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

            static message::slot::cerberus_slot _newSlot(message::slot::BaseSlot::SlotType type);

        public:
            static Cerberus* provider();

            static std::string strPrint(const char* format, ...);

            static CerberusInitParms cerberusDefaultParms();

            //Logging section:                                      ===================================================

            static void log(const std::string& str, LogLevel logLevel = LL_Info);

            //Performs the init sequence of the Cerberus framework. This operation must precede any others
            void init(const CerberusInitParms& parms);

            //Message factory section:                              ===================================================

            //Adds a template of the given message to the register, returning the chosen ID
            uint32_t registerMessage(const message::Message& message, const std::string& name = std::string());

            //Removes a message template with given ID from the register
            void forgetMessage(uint32_t id);

            //Returns a registered message ID searched by its name
            uint32_t messageIdByName(const std::string& name) const;

            //Factory of messages. A call to this method will return an empty but structured message.
            //Will throw an exception if ID was not found.
            message::cerberus_message messageConstruct(uint32_t id) const;

    };
}

#endif // CERBERUS_H
