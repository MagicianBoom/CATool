#ifndef __CATOOL_H
#define __CATOOL_H

typedef struct {
    char *name;
    int (*func)(int, char *[]);
    char *help_info;
} command;

extern int help(int argc, char *argv[]);
extern int exit_catool(int argc, char *argv[]);
extern int hello_world(int argc, char *argv[]);

command commands[] = {
    {"help",        help,        "help"},
    {"exit",        exit_catool, NULL  },
    {"quit",        exit_catool, NULL  },
    {"hello_world", hello_world, NULL  },
    /* Add your command after here! */

    /* End! */
    {NULL,          NULL,        NULL  },
};

#endif
