#ifndef BUILTINS_H
#define BUILTINS_H

#include <stdbool.h>

#include "parsing.h"

int run_builtin(struct parsed_command *cmd);
int set_pwd_if_changed(void);
bool is_builtin(char *command);

struct pwd_memo {
    int changed;
    char *pwd;
};

#endif
