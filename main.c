#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "basic.h"
#include "catool.h"

static struct catool_info catool = {
    .cmd_buffer = {'\0'},
    .cmd_history = {'\0'},
    .cmd_idx = 0,
    .cmd_history_nums = 0,
    .cmd_length = 0,
    .now_cursor = 0,
    .tab_sucess_flag = 0,
    .cmd_idx_history = 0,
};

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

int exit_catool(int argc, char *argv[])
{
    printf("======================== COMMAND AGGREGATOR TOOL BYE! ========================\n");
    exit(0);
}

int hello_world(int argc, char *argv[])
{
    printf("Hello, this is CATool, have a nice day!\n");
    return 0;
}

static void disable_raw_mode(void)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &catool.orig_termios);
}

static void enable_raw_mode(void)
{
    tcgetattr(STDIN_FILENO, &catool.orig_termios);
    atexit(disable_raw_mode);

    struct termios raw = catool.orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

static void clear_line(void)
{
    printf("\r\033[K");
}

static void display_buffer(char *buffer)
{
    clear_line();
    printf("> %s", buffer);
    fflush(stdout);
}

static void handle_backspace(char *buffer, int *cmd_length)
{
    if (*cmd_length > 0) {
        buffer[--(*cmd_length)] = '\0';
    }

    catool.tab_sucess_flag = 0;
}

static void handle_delete(char *buffer, int *cmd_length, int now_cursor)
{
    if (now_cursor < *cmd_length) {
        memmove(buffer + now_cursor, buffer + now_cursor + 1, (*cmd_length) - now_cursor);
        (*cmd_length)--;
    }
}

static int tab_cmp_help(char *buffer, int *cmd_length)
{
    printf("\n");
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strncmp(buffer, commands[i].name, *cmd_length) == 0) {
            printf("  %s\t", commands[i].name);
        }
    }
    printf("\n");
}

static int handle_tab_completion(char *buffer, int *cmd_length)
{
    u32 match_counter = 0;
    u32 match_idx = 0;

    if (strlen(buffer) == 0 || catool.tab_sucess_flag == 1) {
        goto help;
    }

    for (int i = 0; commands[i].name != NULL; i++) {
        if (strncmp(buffer, commands[i].name, *cmd_length) == 0) {
            match_counter++;
            match_idx = i;
        }
    }

    if (match_counter == 1) {
        strcpy(buffer, commands[match_idx].name);
        *cmd_length = strlen(commands[match_idx].name);
        catool.tab_sucess_flag = 1;

        if (catool.cmd_length < MAX_CMD_BUFFER_SIZE - 1) {
            catool.cmd_buffer[catool.cmd_length++] = ' ';
            catool.cmd_buffer[catool.cmd_length] = '\0';
        }
        return 0;
    }

help:
    tab_cmp_help(buffer, cmd_length);
    return -1;
}

static void parse_cmd_buffer(char *cmd_buffer, int *argc, char ***argv)
{
    int count = 0;
    int in_word = 0;
    char *cmd_buf_tmp = malloc(MAX_CMD_BUFFER_SIZE);
    char *ptr = cmd_buf_tmp;

    if (cmd_buf_tmp == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
    }

    memcpy(cmd_buf_tmp, cmd_buffer, MAX_CMD_BUFFER_SIZE);

    // Count the number of arguments
    while (*ptr != '\0') {
        if (*ptr == ' ') {
            in_word = 0;
        } else {
            if (!in_word) {
                count++;
                in_word = 1;
            }
        }
        ptr++;
    }

    // Allocate memory for argc and argv
    *argc = count;
    *argv = (char **)malloc((count + 1) * sizeof(char *));
    if (*argv == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
    }

    // Populate argv with pointers to each argument
    int idx = 0;
    char *arg_start = NULL;
    ptr = cmd_buf_tmp;
    in_word = 0;
    while (*ptr != '\0') {
        if (*ptr == ' ') {
            *ptr = '\0'; // Null terminate the previous argument
            if (in_word) {
                (*argv)[idx] = strdup(arg_start);
                if ((*argv)[idx] == NULL) {
                    fprintf(stderr, "Memory allocation failed.\n");
                    exit(1);
                }
                idx++;
                in_word = 0;
            }
        } else {
            if (!in_word) {
                arg_start = ptr;
                in_word = 1;
            }
        }
        ptr++;
    }

    // Copy the last argument
    if (in_word) {
        (*argv)[idx] = strdup(arg_start);
        if ((*argv)[idx] == NULL) {
            fprintf(stderr, "Memory allocation failed.\n");
            exit(1);
        }
        idx++;
    }

    (*argv)[count] = NULL; // Null terminate argv
    *argc = count;         // Update argc to the actual count
}

static void free_argv(int argc, char *argv[])
{
    if (argv == NULL) {
        return;
    }

    for (int i = 0; i < argc; i++) {
        free(argv[i]);
    }
    free(argv);
}

static int system_cmd(void)
{
    return system(catool.cmd_buffer);
}

static int handle_command(void)
{
    int argc = 0;
    char **argv = NULL;
    int ret = -1;

    parse_cmd_buffer(catool.cmd_buffer, &argc, &argv);
    if (argv[0] == NULL) {
        ret = -1;
        goto done;
    }

    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcmp(argv[0], commands[i].name) == 0) {
            commands[i].func(argc, argv);
            ret = 0;
            goto done;
        }
        ret = -1;
    }

    printf("%s: CATool command not found...\n", argv[0]);
    printf("%s: Start to find system command to process...\n", argv[0]);

    ret = system_cmd();

done:
    free_argv(argc, argv);
    return ret;
}

static void update_cmd_history(void)
{
    catool.cmd_idx = (catool.cmd_idx) % MAX_CMD_HISTORY_NUMS;
    memcpy(catool.cmd_history[catool.cmd_idx], catool.cmd_buffer, MAX_CMD_BUFFER_SIZE);
    if ((catool.cmd_idx + 1) >= MAX_CMD_HISTORY_NUMS) {
        catool.cmd_idx = 0;
    } else {
        catool.cmd_idx++;
    }

    if (catool.cmd_history_nums < (MAX_CMD_HISTORY_NUMS - 1)) {
        catool.cmd_history_nums++;
    }
}

static void update_cmd_buffer(u32 cmd_idx)
{

    memcpy(catool.cmd_buffer, catool.cmd_history[cmd_idx], MAX_CMD_BUFFER_SIZE);
}

static int handle_enter(void)
{
    int ret = 0;

    ret = handle_command();
    if (!ret) {
        update_cmd_history();
        memset(catool.cmd_history[catool.cmd_idx], '\0', MAX_CMD_BUFFER_SIZE);
    }

    catool.cmd_length = 0;
    catool.now_cursor = 0;
    catool.tab_sucess_flag = 0;
    catool.cmd_idx_history = catool.cmd_idx;
    memset(catool.cmd_buffer, '\0', MAX_CMD_BUFFER_SIZE);

    return 0;
}

static u32 get_next_cmd_idx(u32 now_idx)
{
    u32 idx = 0;

    if (now_idx == catool.cmd_history_nums) {
        idx = 0;
    } else {
        idx = now_idx + 1;
    }

    return idx;
}

static u32 get_last_cmd_idx(u32 now_idx)
{
    u32 idx = 0;

    if (now_idx == 0) {
        idx = catool.cmd_history_nums;
    } else {
        idx = now_idx - 1;
    }

    return idx;
}

static int handle_up_arrow_key(void)
{
    int ret = 0;

    catool.cmd_idx_history = get_last_cmd_idx(catool.cmd_idx_history);
    if (catool.cmd_idx_history == catool.cmd_idx) {
        catool.cmd_idx_history = get_next_cmd_idx(catool.cmd_idx_history);
    }
    update_cmd_buffer(catool.cmd_idx_history);

    catool.cmd_length = strlen(catool.cmd_buffer);
    catool.now_cursor = catool.cmd_length - 1;

    return 0;
}

static int handle_down_arrow_key(void)
{
    int ret = 0;

    if (catool.cmd_idx_history != catool.cmd_idx) {
        catool.cmd_idx_history = get_next_cmd_idx(catool.cmd_idx_history);
    }
    update_cmd_buffer(catool.cmd_idx_history);

    catool.cmd_length = strlen(catool.cmd_buffer);
    catool.now_cursor = catool.cmd_length - 1;

    return 0;
}

static void parse_and_execute(void)
{
    int ret = 0;
    catool.cmd_idx_history = catool.cmd_idx;

    memset(catool.cmd_buffer, '\0', MAX_CMD_BUFFER_SIZE);

    while (1) {
        char c;
        read(STDIN_FILENO, &c, 1);

        if (c == 3) { // Ctrl+C to exit
            break;
        } else if (c == '\n') { // Enter key
            printf("\n");
            if (strcmp(catool.cmd_buffer, "exit") == 0) {
                exit_catool(0, NULL);
                break;
            }

            handle_enter();
        } else if (c == 127 || c == 8) { // Backspace key
            handle_backspace(catool.cmd_buffer, &catool.cmd_length);
        } else if (c == '\t') { // Tab key
            handle_tab_completion(catool.cmd_buffer, &catool.cmd_length);
        } else if (c == 27) { // Arrow keys (and other escape sequences)
            catool.tab_sucess_flag = 0;
            char seq[3];
            if (read(STDIN_FILENO, &seq[0], 1) == 0) {
                continue;
            }

            if (read(STDIN_FILENO, &seq[1], 1) == 0) {
                continue;
            }

            if (seq[0] == '[') {
                if (seq[1] == '3' && read(STDIN_FILENO, &seq[2], 1) == 1 && seq[2] == '~') {
                    handle_delete(catool.cmd_buffer, &catool.cmd_length, catool.now_cursor);
                }

                if (seq[1] == 'A') { // up arrow
                    handle_up_arrow_key();
                }

                if (seq[1] == 'B') { // down arrow
                    handle_down_arrow_key();
                }
            }
        } else if (isprint(c)) {
            if (catool.cmd_length < MAX_CMD_BUFFER_SIZE - 1) {
                catool.cmd_buffer[catool.cmd_length++] = c;
                catool.cmd_buffer[catool.cmd_length] = '\0';
            }
        }
        display_buffer(catool.cmd_buffer);
    }
}

static void on_ctrl_c(int sig)
{
    memset(catool.cmd_buffer, '\0', MAX_CMD_BUFFER_SIZE);
    printf("^C\n");
    display_buffer(catool.cmd_buffer);
    catool.cmd_length = 0;
    catool.now_cursor = 0;
    catool.cmd_idx_history = catool.cmd_idx;
}

int main(void)
{
    printf("======================== WELCOME TO COMMAND AGGREGATOR TOOL! ========================\n");
    signal(SIGINT, on_ctrl_c);
    enable_raw_mode();
    display_buffer(catool.cmd_buffer);
    parse_and_execute();
    disable_raw_mode();
    return 0;
}
