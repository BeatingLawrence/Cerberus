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
//Objects:

#define CERBERUS_INVALID_ID             0u
#define CERBERUS_OBJECT_THREAD          1u
#define CERBERUS_OBJECT_MESSAGETMPLT    2u
#define CERBERUS_OBJECT_SOCKET          3u

//Filesystem

#define CERBERUS_FILE_WRITE     ((char)0b00000001)    //File handler open with writing capability
#define CERBERUS_FILE_READ      ((char)0b00000010)    //File handler open with reading capability
#define CERBERUS_FILE_BINARY    ((char)0b00000100)    //File handler operations in binary mode
#define CERBERUS_FILE_EOF       ((char)0b00001000)    //File handler open with cursor setted to end of file
#define CERBERUS_FILE_APPEND    ((char)0b00010000)    //File handler writing operations happen always at the end of file (log style)
#define CERBERUS_FILE_TRUNCATE  ((char)0b00100000)    //File content cleared while the handler is open

#endif // CERBERUS_DEFINE_H
