#ifndef CERBERUS_H
#define CERBERUS_H

#include <string>
#include "Cerberus_global.h"

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

        private:
            Cerberus();

            bool _isColorSupported();

            bool m_useColorInTerminal;

            std::string m_stdout_terminalFormatting;

            std::string m_stderr_terminalFormatting;

            std::string _parseFormattingData(const TerminalFormatting& data);

            bool m_initFlag;

        public:
            struct CerberusInitParms
            {
                bool terminalFormattingDisabled;
                TerminalFormatting stdout_formatting;
                TerminalFormatting stderr_formatting;
            };

            static Cerberus* provider();

            static std::string strPrint(const char* format, ...);

            static void stdoutPrint(const std::string& str);

            static void stderrPrint(const std::string& str);

            void init(const CerberusInitParms& parms);
    };
}
#endif // CERBERUS_H
