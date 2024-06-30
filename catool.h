#ifndef __CATOOL_H
#define __CATOOL_H

typedef struct {
    char *name;
    int (*func)(int, char *[]);
    char *help_info;
} command;

extern int help(int argc, char *argv[]);
extern int exit_pcietool(int argc, char *argv[]);
extern int hello_world(int argc, char *argv[]);

command commands[] = {
    {"help",        help,          "help"},
    {"exit",        exit_pcietool, NULL  },
    {"quit",        exit_pcietool, NULL  },
    {"hello_world", hello_world,   NULL  },
    /* Add your command after here! */

    /* End! */
    {NULL,          NULL,          NULL  },
};

#endif
