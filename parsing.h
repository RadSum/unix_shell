#ifndef PARSING_H
#define PARSING_H

#include <unistd.h>

struct parsed_command {
    char *command;
    char **argv;
    int stdin_fd;
    int stdout_fd;
    pid_t pid;
};

char **parse_line_with_pipes(char *line, int *cmd_count);
struct parsed_command *parse_commands(char **parsed_line, int cmd_count);
void free_parsed_commands(struct parsed_command *cmds, int cmd_count);

#endif /* PARSING_H */
