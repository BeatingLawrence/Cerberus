#ifndef TYPES_H
#define TYPES_H

#include <cstdint>

struct CerberusLogRole
{
    uint8_t textFormatting[3];  //up to 3 formatting specifiers, 0 will be ignored, see define.h
    uint8_t foregroundColor;    //color specifier
    uint8_t backgroundColor;    //color specifier
};

struct CerberusLogSetup
{
    bool disableFormatting;
    char* logFileName;
    CerberusLogRole infoRole;
    CerberusLogRole warningRole;
    CerberusLogRole errorRole;
    CerberusLogRole debugRole;
};

struct CerberusInitParms
{
    CerberusLogSetup logSetup;
    //add more configuration members here..
};


#endif // TYPES_H
