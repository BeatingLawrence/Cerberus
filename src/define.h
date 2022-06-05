#ifndef CERBERUS_DEFINE_H
#define CERBERUS_DEFINE_H

#ifdef WINDOWS_SYSTEM

    #include <windows.h>

    #define TERMINAL_FOREGROUND_RED         FOREGROUND_RED
    #define TERMINAL_FOREGROUND_GREEN       FOREGROUND_GREEN
    #define TERMINAL_FOREGROUND_BLUE        FOREGROUND_BLUE
    #define TERMINAL_FOREGROUND_INTENSITY   FOREGROUND_INTENSITY

    #define TERMINAL_BACKGROUND_RED         BACKGROUND_RED
    #define TERMINAL_BACKGROUND_GREEN       BACKGROUND_GREEN
    #define TERMINAL_BACKGROUND_BLUE        BACKGROUND_BLUE
    #define TERMINAL_BACKGROUND_INTENSITY   BACKGROUND_INTENSITY

#else

    //Terminal colors specifiers:

    #define TERMINAL_FOREGROUND_BLACK   30
    #define TERMINAL_FOREGROUND_RED     31
    #define TERMINAL_FOREGROUND_GREEN   32
    #define TERMINAL_FOREGROUND_YELLOW  33
    #define TERMINAL_FOREGROUND_BLUE    34
    #define TERMINAL_FOREGROUND_MAGENTA 35
    #define TERMINAL_FOREGROUND_CYAN    36
    #define TERMINAL_FOREGROUND_WHITE   37

    #define TERMINAL_BACKGROUND_BLACK   40
    #define TERMINAL_BACKGROUND_RED     41
    #define TERMINAL_BACKGROUND_GREEN   42
    #define TERMINAL_BACKGROUND_YELLOW  43
    #define TERMINAL_BACKGROUND_BLUE    44
    #define TERMINAL_BACKGROUND_MAGENTA 45
    #define TERMINAL_BACKGROUND_CYAN    46
    #define TERMINAL_BACKGROUND_WHITE   47

    //Terminal formatting specifiers:

    #define TERMINAL_RESET              0
    #define TERMINAL_BOLD               1
    #define TERMINAL_UNDERLINE          4
    #define TERMINAL_INVERSE            7

#endif

//Cerberus constants definitions
//Standard Messages (1~99 are reserved and not used by factory)
#define CERBERUS_FACTORY_START_ID           100u

#define CERBERUS_INVALID_ID                 0u
#define CERBERUS_MESSAGE_SHUTDOWN_ID        1u
#define CERBERUS_MESSAGE_LOG_ID             2u

//Filesystem

//Note: cannot combine TRUNCATE and APPEND. cannot set TRUNCATE without WRITE

#define CERBERUS_FILE_WRITE     ((char)0b00000001)    //File handler with writing capability
#define CERBERUS_FILE_BINARY    ((char)0b00000010)    //File handler operations in binary mode
#define CERBERUS_FILE_EOF       ((char)0b00000100)    //File handler with cursor setted to end of file when open
#define CERBERUS_FILE_APPEND    ((char)0b00001000)    //File handler write operations happen always at the end of file (log style)
#define CERBERUS_FILE_TRUNCATE  ((char)0b00010000)    //File content cleared when open

#endif // CERBERUS_DEFINE_H
