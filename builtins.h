#ifndef BUILTINS_H
#define BUILTINS_H

int run_builtin(char *cmd, char *argv[]);

int set_pwd_if_changed(void);

struct pwd_memo {
    int changed;
    char *pwd;
};

#endif
