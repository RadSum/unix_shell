#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "builtins.h"

static int run_command(char *command, char *argv[])
{
    pid_t process = fork();
    if (process == -1) 
        return -1;

    if (process == 0) {
        execvp(command, argv);

        //error happened
        exit(127);
    }

    int status;
    if (waitpid(process, &status, 0) == -1) 
        return -1;

    if (WEXITSTATUS(status) == 127) {
        fprintf(stderr, "There was an error running the command `%s`\n", command);
    }

    return 0;
}

#define MAX_ARGV_SIZE 16

int main(void)
{
    char *user = getenv("USER");
    if (user == NULL) {
        fprintf(stderr, "There was an error getting the USER variable");
        exit(1);
    }

    char *prompt = getenv("PROMPT");
    if (prompt == NULL) {
        prompt = malloc(strlen(user) + 2);
        strcpy(prompt, user);
        strcpy(prompt + strlen(user), "$");
    }

    char *parsed_command[MAX_ARGV_SIZE];

    char *line_read = NULL;
    size_t line_size = 0;
    while (1) {
        printf("%s ", prompt);
        ssize_t bytes_read = getline(&line_read, &line_size, stdin);
        if (bytes_read == -1) {
            fprintf(stderr, "There was an error reading a line from stdin\n");
            exit(1);
        }
        // don't include the `\n` character
        line_read[bytes_read - 1] = '\0';

        if (strcmp(line_read, "exit") == 0) {
            exit(0);
        }

        char *str = strtok(line_read, " ");
        parsed_command[0] = str == NULL ? line_read : str;
        int c = 1;
        while ((str = strtok(NULL, " ")) != NULL) {
            parsed_command[c++] = str; 
        }
        parsed_command[c] = NULL;

        int status = run_builtin(parsed_command[0], parsed_command);
        if (status == -1) 
            exit(1); 
        if (status != -3)
            continue;

        if (run_command(parsed_command[0], parsed_command) == -1) {
            exit(1);
        }
    }

    return 0;
}
