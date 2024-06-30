#ifndef __BASIC_H
#define __BASIC_H

#include <termios.h>

#include "catool.h"
#include "catool_conf.h"

typedef unsigned long long int u64;
typedef signed long long int s64;

typedef unsigned int u32;
typedef signed int s32;

typedef unsigned short int u16;
typedef signed short int s16;

typedef unsigned char u8;
typedef signed char s8;

struct catool_info {
    struct termios orig_termios;
    char cmd_buffer[MAX_CMD_BUFFER_SIZE];
    char cmd_history[MAX_CMD_HISTORY_NUMS][MAX_CMD_BUFFER_SIZE];
    u32 cmd_idx;
    u32 cmd_idx_history;
    u32 cmd_history_nums;
    u32 cmd_length;
    int now_cursor;
    u32 tab_sucess_flag;
};

#define TO_STR(def)  #def
#define DEFINE_2_STR(def)  TO_STR(def)

#endif
