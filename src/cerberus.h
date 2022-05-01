#ifndef CERBERUS_H
#define CERBERUS_H

#include <string>
#include "Cerberus_global.h"
#include "./register.h"
#include "./mutex/mutex.h"

namespace cerberus
{
    class CERBERUS_EXPORT Cerberus
    {
        public:
            struct TerminalFormatting
            {
                uint8_t textFormatting[3];  //up to 3 formatting specifiers, 0 will be ignored, see define.h
                uint8_t foregroundColor;    //color specifier
                uint8_t backgroundColor;    //color specifier
            };

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

            bool m_useColorInTerminal;

            std::string m_stdout_terminalFormatting;

            std::string m_stderr_terminalFormatting;

            std::string _parseFormattingData(const TerminalFormatting& data);

            static mutex::Mutex m_logMutex;

            bool m_initFlag;

            Register m_register;

            static message::slot::cerberus_slot _newSlot(message::slot::BaseSlot::SlotType type);

        public:
            struct CerberusInitParms
            {
                bool terminalFormattingDisabled;
                TerminalFormatting stdout_formatting;
                TerminalFormatting stderr_formatting;
            };

            static Cerberus* provider();

            static std::string strPrint(const char* format, ...);

            //Logging section:                                      ===================================================

            static void log(const std::string& str, LogLevel logLevel = LL_Info);

            static void stdoutPrint(const std::string& str);

            static void stderrPrint(const std::string& str);

            void init(const CerberusInitParms& parms);

            //Message factory section:                              ===================================================

            uint32_t registerMessage(const message::Message& message, const std::string& name = std::string());

            void forgetMessage(uint32_t id);

            uint32_t messageIdByName(const std::string& name) const;

            //Factory of messages. A call to this method will return an empty but structured message.
            //Will throw an exception if ID was not found.
            message::cerberus_message messageConstruct(uint32_t id) const;

    };
}
#endif // CERBERUS_H
