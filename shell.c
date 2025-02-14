#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#include "builtins.h"
#include "parsing.h"

static int run_command(struct parsed_command *cmd)
{
    pid_t process = fork();
    if (process == -1) 
        return -1;

    if (process == 0) {
        if (cmd->stdin_fd != -1) {
            // check for errors 
            dup2(cmd->stdin_fd, STDIN_FILENO);
        }
        if (cmd->stdout_fd != -1) {
            // check for errors 
            dup2(cmd->stdout_fd, STDOUT_FILENO);
        }

        execvp(cmd->argv[0], cmd->argv);
        // execvp failed, so exit with errno
        exit(errno);
    }
    cmd->pid = process;
    if (cmd->stdout_fd != -1)
        close(cmd->stdout_fd);
    if (cmd->stdin_fd != -1)
        close(cmd->stdin_fd);
    return 0;
}

static void wait_for_processes(struct parsed_command *cmds, int cmd_count)
{
    int rem = cmd_count;
    while (rem > 0) {
        for (int i = 0; i < cmd_count; ++i) {
            if (cmds[i].pid < 0) {
                if (cmds[i].pid == -2) {
                    rem--;
                    cmds[i].pid = -1;
                }
                continue;
            }
            int status;
            if (waitpid(cmds[i].pid, &status, WNOHANG) == 0) {
            } else {
                int exit_status = WEXITSTATUS(status);
                if (exit_status & EACCES)
                    fprintf(stderr, "Permission to execute %s is not permitted\n", cmds[i].command);
                if (exit_status & ENOENT)
                    fprintf(stderr, "%s: command not found\n", cmds[i].command);
                cmds[i].pid = -1;
                rem--;
            }
        }
    }
}

extern struct pwd_memo pwdm;

#define PROMPT_BUFFER_SIZE 256

int main(void)
{
    char *user = getenv("USER");
    if (user == NULL) {
        fprintf(stderr, "There was an error getting the USER variable");
        exit(1);
    }

    char *line_read = NULL;
    size_t line_size = 0;
    while (1) {
        if (set_pwd_if_changed() == -1) 
            exit(1);
        printf("%s$ ", pwdm.pwd);
        ssize_t bytes_read = getline(&line_read, &line_size, stdin);
        if (bytes_read == -1) {
            fprintf(stderr, "There was an error reading a line from stdin\n");
            exit(1);
        }
        // don't include the `\n` character
        line_read[bytes_read - 1] = '\0';

        int cmd_count = 0;
        char **parsed_pipes = parse_line_with_pipes(line_read, &cmd_count);
        if (parsed_pipes == NULL) {
            fprintf(stderr, "There was an error parsing the line\n");
            exit(1);
        }
        struct parsed_command *cmds = parse_commands(parsed_pipes, cmd_count);

        for (int i = 0; i < cmd_count; ++i) {
            if (is_builtin(cmds[i].argv[0])) {
                if (run_builtin(cmds + i) == -1)
                    exit(1);
            } else {
                if (run_command(cmds + i) == -1)
                    exit(1);
            }
        }
        wait_for_processes(cmds, cmd_count);
        
        free_parsed_commands(cmds, cmd_count);
        free(parsed_pipes);
    }
    free(line_read);
    free(pwdm.pwd);

    return 0;
}
