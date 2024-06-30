#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "catool.h"

extern command commands[];

int ly_test(int argc, char **argv)
{
    for (int i = 0; i < argc; i++) {
        printf("LY_DEBUG===> argv[%d]: %s\n", i, argv[i]);
    }

    return 0;
}

int help(int argc, char *argv[])
{
    printf("Available commands:\n");
    for (int i = 0; commands[i].name != NULL; i++) {
        if (commands[i].help_info) {
            printf("  %-20s %s\n", commands[i].name, commands[i].help_info);
        } else {
            printf("  %-20s\n", commands[i].name);
        }
    }
}

int exit_pcietool(int argc, char *argv[])
{
    printf("======================== COMMAND AGGREGATOR TOOL BYE! ========================\n");
    exit(0);
}
