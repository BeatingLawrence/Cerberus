#ifndef CERBERUS_DEFINE_H
#define CERBERUS_DEFINE_H

#ifdef WINDOWS_SYSTEM

#include <windows.h>

#define TERMINAL_FOREGROUND_RED FOREGROUND_RED
#define TERMINAL_FOREGROUND_GREEN FOREGROUND_GREEN
#define TERMINAL_FOREGROUND_BLUE FOREGROUND_BLUE
#define TERMINAL_FOREGROUND_INTENSITY FOREGROUND_INTENSITY

#define TERMINAL_BACKGROUND_RED BACKGROUND_RED
#define TERMINAL_BACKGROUND_GREEN BACKGROUND_GREEN
#define TERMINAL_BACKGROUND_BLUE BACKGROUND_BLUE
#define TERMINAL_BACKGROUND_INTENSITY BACKGROUND_INTENSITY

#else

// Terminal colors specifiers:

#define TERMINAL_FOREGROUND_BLACK 30
#define TERMINAL_FOREGROUND_RED 31
#define TERMINAL_FOREGROUND_GREEN 32
#define TERMINAL_FOREGROUND_YELLOW 33
#define TERMINAL_FOREGROUND_BLUE 34
#define TERMINAL_FOREGROUND_MAGENTA 35
#define TERMINAL_FOREGROUND_CYAN 36
#define TERMINAL_FOREGROUND_WHITE 37

#define TERMINAL_BACKGROUND_BLACK 40
#define TERMINAL_BACKGROUND_RED 41
#define TERMINAL_BACKGROUND_GREEN 42
#define TERMINAL_BACKGROUND_YELLOW 43
#define TERMINAL_BACKGROUND_BLUE 44
#define TERMINAL_BACKGROUND_MAGENTA 45
#define TERMINAL_BACKGROUND_CYAN 46
#define TERMINAL_BACKGROUND_WHITE 47

// Terminal formatting specifiers:

#define TERMINAL_RESET 0
#define TERMINAL_BOLD 1
#define TERMINAL_UNDERLINE 4
#define TERMINAL_INVERSE 7

#endif

#define hashFunc(str) CerberusUtils::hash_fnv1a(str)                             // generic hashing function
#define hashFunc_res(str) core::CerberusRegister::removeReserved(hashFunc(str))  // set to zero the LS byte

// Cerberus constants definitions
// Standard Messages with id 1~255 are reserved and not used by register

#define CERBERUS_INVALID_ID 0u
#define CERBERUS_REG_RANGE_START 256u  // the first ID outside the reserved range

// Default templates:
#define CERBERUS_MESSAGE_TERM_ID 1u        // sent to threads to signal termination
#define CERBERUS_MESSAGE_LOG_ID 2u         // sent to the logger thread to log on file
#define CERBERUS_MESSAGE_TASK_ID 3u        // sent to a player to start a task
#define CERBERUS_MESSAGE_TASKEND_ID 4u     // sent from a player to signal a task end
#define CERBERUS_MESSAGE_SOCKETDATA_ID 5u  // sent by cerberus to listening threads

#endif  // CERBERUS_DEFINE_H
