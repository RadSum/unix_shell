#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>

#include "parsing.h"

#define MAX_ARGV_SIZE 16

static int count_pipes(char *str)
{
    int c = 0;
    for (size_t i = 0; i < strlen(str); ++i) {
        if (str[i] == '|')
            ++c;
    }
    return c;
}

char **parse_line_with_pipes(char *line, int *cmd_count)
{
    const int size = count_pipes(line) + 2;
    char **parsed = malloc(size * sizeof(char *));
    if (parsed == NULL)
        return NULL;
    parsed[size - 1] = NULL;

    int c = 0;
    parsed[c++] = line;
    const int l = strlen(line);
    for (size_t i = 0; i < l; ++i) {
        if (line[i] == '|') {
            line[i] = '\0';
            parsed[c++] = line + i + 1;
        }
    }
    *cmd_count = size - 1;

    return parsed;
}

struct parsed_command *parse_commands(char **parsed_line, int cmd_count)
{
    struct parsed_command *cmds = malloc(cmd_count * sizeof(struct parsed_command));
    if (cmds == NULL) 
        return NULL;

    for (int i = 0; i < cmd_count; ++i) {
        const int len = strlen(parsed_line[i]);
        int j = 0;
        while (parsed_line[i][j++] == ' ');
        char *dupped = strdup(parsed_line[i] + j - 1);
        if (dupped == NULL)
            return NULL;
        cmds[i].command = dupped;
        cmds[i].argv = malloc(sizeof(char *) * MAX_ARGV_SIZE);

        char *str = strtok(dupped, " ");
        cmds[i].argv[0] = str == NULL ? dupped : str;
        int c = 1;
        while ((str = strtok(NULL, " ")) != NULL) {
            cmds[i].argv[c++] = str; 
        }
        cmds[i].argv[c] = NULL;

        cmds[i].stdin_fd = cmds[i].stdout_fd = -1;
        if (i == 0)
            continue;

        int pipes[2]; 
        if (pipe(pipes) == -1) 
            return NULL; // better error handling
        cmds[i - 1].stdout_fd = pipes[1];
        cmds[i].stdin_fd = pipes[0];
    }
    return cmds;
}

void free_parsed_commands(struct parsed_command *cmds, int cmd_count)
{
    for (int i = 0; i < cmd_count; ++i) {
        free(cmds[i].command); 
        free(cmds[i].argv);
    }
    free(cmds);
}

