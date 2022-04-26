#ifndef CERBERUS_DEFINE_H
#define CERBERUS_DEFINE_H

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

//===========================

#define SLOT_INVALID_ID     0
#define SLOT_UCHAR_ID       1
#define SLOT_CHAR_ID        2
#define SLOT_USHORT_ID      3
#define SLOT_SHORT_ID       4
#define SLOT_ULONG_ID       5
#define SLOT_LONG_ID        6
#define SLOT_ULONGLONG_ID   7
#define SLOT_LONGLONG_ID    8
#define SLOT_FLOAT_ID       9
#define SLOT_DOUBLE_ID      10
#define SLOT_BOOL_ID        11

#endif // CERBERUS_DEFINE_H
