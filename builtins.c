#include "builtins.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

typedef int (*builtin_function)(char*[]);

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
    
    if (pwdm.pwd != NULL) 
        free(pwdm.pwd);

    char *p = strdup(buffer);
    if (p == NULL) 
        return -2;
    if (pwdm.pwd != NULL) 
        free(pwdm.pwd);
    pwdm.pwd = p;
    pwdm.changed = 0;
    puts("changed pwd memo");
    return 0;
}

static int exit_builtin(char *argv[])
{
    if (argv[1] == NULL) 
        exit(0);

    int exit_number = atoi(argv[1]);
    if (exit_number == 0) {
        fprintf(stderr, "Usage: exit `exit_number`\n");
        return -2;
    }

    exit(exit_number);
}

static int cd(char *argv[])
{
    if (argv[1] == NULL) {
        char *user_home = getenv("HOME");
        if (user_home == NULL) 
            return -2;
        argv[1] = user_home;
    }
    // -2 means that recovarable error happened 
    // -1 means that unrecovarable error happened 
    // 0 means ok
    if (chdir(argv[1]) == -1) {
        switch (errno) {
        case ENOENT:
            fprintf(stderr, "%s does not exist\n", argv[1]);
            return -2;
        case EACCES:
            fprintf(stderr, "Permission denied on %s\n", argv[1]);
            return -2;
        case ENOTDIR:
            fprintf(stderr, "%s is not a directory\n", argv[1]);
            return -2;
        }
        return -1;
    } 
    pwdm.changed = 1;
    return 0;
}

static int pwd(char *argv[])
{
    (void)pwd;

    if (pwdm.changed == 0) {
        puts(pwdm.pwd);
        return 0;
    }

    if (set_pwd_if_changed() == -1) 
        return -1;

    puts(pwdm.pwd);

    return 0;
}

static int type(char *argv[])
{
    if (argv[1] == NULL) {
        fprintf(stderr, "Usage: type `command`\n"); 
        return 0; // 0 for now
    }

    printf("%s is %s a shell builtin\n", argv[1], get_builtin(argv[1]) == NULL ? "not" : "\x08");
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

int run_builtin(char *command, char *argv[])
{ 
    struct builtin *builtin = get_builtin(command);
    if (builtin == NULL)
        return -3;
    return builtin->func(argv);
}

