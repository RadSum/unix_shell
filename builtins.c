#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include "builtins.h"
#include "parsing.h"

typedef int (*builtin_function)(struct parsed_command*);

struct builtin {
    builtin_function func;
    const char *name;
};

struct pwd_memo pwdm = { .changed = 1, .pwd = NULL };

static struct builtin *get_builtin(char *cmd);

int set_pwd_if_changed(void)
{
    if (pwdm.changed == 0)
        return 0;

    const int path_max = pathconf(".", _PC_PATH_MAX);
    char buffer[path_max];
    char *current_dir = getcwd(buffer, path_max);
    if (current_dir == NULL) {
        fprintf(stderr, "There was an error getting the current path\n");
        return -1;
    }

    char *p = strdup(buffer);
    if (p == NULL) 
        return -2;
    if (pwdm.pwd != NULL) 
        free(pwdm.pwd);
    pwdm.pwd = p;
    pwdm.changed = 0;
    return 0;
}

static int exit_builtin(struct parsed_command *cmd)
{
    if (cmd->argv[1] == NULL) 
        exit(0);

    int exit_number = atoi(cmd->argv[1]);
    if (exit_number == 0) {
        fprintf(stderr, "Usage: exit `exit_number`\n");
        return -2;
    }

    exit(exit_number);
}

static int cd(struct parsed_command *cmd)
{
    if (cmd->argv[1] == NULL) {
        char *user_home = getenv("HOME");
        if (user_home == NULL) 
            return -2;
        cmd->argv[1] = user_home;
    }
    if (chdir(cmd->argv[1]) == -1) {
        switch (errno) {
        case ENOENT:
            fprintf(stderr, "%s does not exist\n", cmd->argv[1]);
            return -2;
        case EACCES:
            fprintf(stderr, "Permission denied on %s\n", cmd->argv[1]);
            return -2;
        case ENOTDIR:
            fprintf(stderr, "%s is not a directory\n", cmd->argv[1]);
            return -2;
        }
        return -1;
    } 
    pwdm.changed = 1;
    return 0;
}

static int pwd(struct parsed_command *cmd)
{
    (void)cmd;

    int real_out = cmd->stdout_fd == -1 ? STDOUT_FILENO : cmd->stdout_fd;
    if (pwdm.changed == 0) {
        dprintf(real_out, "%s\n", pwdm.pwd);
        return 0;
    }

    if (set_pwd_if_changed() == -1) 
        return -1;

    dprintf(real_out, "%s\n", pwdm.pwd);

    return 0;
}

static int type(struct parsed_command *cmd)
{
    if (cmd->argv[1] == NULL) {
        fprintf(stderr, "Usage: type `command`\n"); 
        return 0; // 0 for now
    }
    int real_out = cmd->stdout_fd == -1 ? STDOUT_FILENO : cmd->stdout_fd;
    dprintf(real_out, "%s is %s a shell builtin\n", cmd->argv[1], get_builtin(cmd->argv[1]) == NULL ? "not" : "\x08");
    return 0;
}

static struct builtin builtins[] = 
{ 
    { .func = &exit_builtin, .name = "exit" }, 
    { .func = &cd, .name = "cd" },  
    { .func = &pwd, .name = "pwd" },
    { .func = &type, .name = "type" },
};

static struct builtin *get_builtin(char *command)
{
    const int number_of_builtins = sizeof(builtins) / sizeof(struct builtin);
    for (int i = 0; i < number_of_builtins; ++i) {
        if (strcmp(command, builtins[i].name) == 0) 
            return builtins + i;
    }
    return NULL;
}

bool is_builtin(char *command)
{
    return get_builtin(command) != NULL;
}

int run_builtin(struct parsed_command *cmd)
{ 
    struct builtin *builtin = get_builtin(cmd->argv[0]);
    cmd->pid = -2;
    int ret = builtin->func(cmd);
    if ((cmd->stdout_fd != -1 && close(cmd->stdout_fd) == -1) || (cmd->stdin_fd != -1 && close(cmd->stdin_fd) == -1))
        fprintf(stderr, "There was an error closing a pipe");
    return ret;
}

