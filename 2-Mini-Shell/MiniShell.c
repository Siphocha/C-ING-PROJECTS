#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGUMENTS 64
#define MAX_PIPES 10

//Built-in command functions
int shell_exit(char **args);
int shell_cd(char **args);
int shell_help(char **args);

//List of built-in commands
char *builtin_str[] = {
    "exit",
    "cd",
    "help"
};

// Array of function pointers for built-in commands
int (*builtin_func[]) (char **) = {
    &shell_exit,
    &shell_cd,
    &shell_help
};

int num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

// Built-in command implementations
int shell_exit(char **args) {
    return 0;
}

int shell_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "shell: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("shell");
        }
    }
    return 1;
}

int shell_help(char **args) {
    printf("Mini Shell with Built-in Commands\n");
    printf("Supports basic Unix commands, I/O redirection, and piping.\n");
    printf("Built-in commands:\n");

    for (int i = 0; i < num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }

    return 1;
}

// Execute a single command (no pipes)
int execute_command(char **args, char *input_file, char *output_file) {
    // Check for built-in commands
    for (int i = 0; i < num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    pid_t pid = fork();
    if (pid == 0) {
        // Child process

        // Handle input redirection
        if (input_file != NULL) {
            int fd = open(input_file, O_RDONLY);
            if (fd < 0) {
                perror("shell");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        // Handle output redirection
        if (output_file != NULL) {
            int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("shell");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        // Execute command
        if (execvp(args[0], args) == -1) {
            perror("shell");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Fork error
        perror("shell");
    } else {
        // Parent process
        int status;
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

// Execute a pipeline of commands
int execute_pipeline(char ***commands, int num_commands) {
    int i;
    int in = 0;
    int fd[2];
    pid_t pid;

    for (i = 0; i < num_commands - 1; i++) {
        pipe(fd);

        pid = fork();
        if (pid == 0) {
            // Child process
            dup2(in, STDIN_FILENO);
            if (i < num_commands - 1) {
                dup2(fd[1], STDOUT_FILENO);
            }
            close(fd[0]);

            if (execvp(commands[i][0], commands[i]) == -1) {
                perror("shell");
                exit(EXIT_FAILURE);
            }
        } else if (pid < 0) {
            perror("shell");
            return 0;
        }

        close(fd[1]);
        in = fd[0];
    }

    // Last command
    pid = fork();
    if (pid == 0) {
        dup2(in, STDIN_FILENO);

        if (execvp(commands[i][0], commands[i]) == -1) {
            perror("shell");
            exit(EXIT_FAILURE);
        }
    } else if (pid < 0) {
        perror("shell");
        return 0;
    }

    // Wait for all children
    for (i = 0; i < num_commands; i++) {
        wait(NULL);
    }

    return 1;
}

// Parse a command line into arguments
char **parse_command(char *line, char **input_file, char **output_file) {
    int position = 0;
    char **tokens = malloc(MAX_ARGUMENTS * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "shell: allocation error\n");
        exit(EXIT_FAILURE);
    }

    // Check for input redirection
    char *input_redirect = strchr(line, '<');
    if (input_redirect != NULL) {
        *input_redirect = '\0';
        *input_file = strtok(input_redirect + 1, " \t\n");
    }

    // Check for output redirection
    char *output_redirect = strchr(line, '>');
    if (output_redirect != NULL) {
        *output_redirect = '\0';
        *output_file = strtok(output_redirect + 1, " \t\n");
    }

    // Parse the remaining command
    token = strtok(line, " \t\n");
    while (token != NULL && position < MAX_ARGUMENTS - 1) {
        tokens[position] = token;
        position++;
        token = strtok(NULL, " \t\n");
    }
    tokens[position] = NULL;

    return tokens;
}

//Parsing a pipeline of commands
int parse_pipeline(char *line, char ***commands[]) {
    int num_commands = 0;
    char *command_str[MAX_PIPES];
    char *saveptr;

    //Split the line into individual commands
    command_str[0] = strtok_r(line, "|", &saveptr);
    while (command_str[num_commands] != NULL && num_commands < MAX_PIPES - 1) {
        num_commands++;
        command_str[num_commands] = strtok_r(NULL, "|", &saveptr);
    }

    //Allocate memory for commands array
    *commands = malloc(num_commands * sizeof(char **));
    if (!*commands) {
        fprintf(stderr, "shell: allocation error\n");
        exit(EXIT_FAILURE);
    }

    // Parse each command
    for (int i = 0; i < num_commands; i++) {
        char *input_file = NULL;
        char *output_file = NULL;

        (*commands)[i] = parse_command(command_str[i], &input_file, &output_file);

        // Check for redirection in piped commands (not allowed)
        if (input_file != NULL || output_file != NULL) {
            fprintf(stderr, "shell: redirection not allowed in piped commands\n");
            return 0;
        }
    }

    return num_commands;
}

// Read a line from stdin
char *read_line() {
    char *line = NULL;
    size_t bufsize = 0;

    if (getline(&line, &bufsize, stdin) == -1) {
        if (feof(stdin)) {
            exit(EXIT_SUCCESS);
        } else {
            perror("shell");
            exit(EXIT_FAILURE);
        }
    }

    return line;
}

// Main shell loop
void shell_loop() {
    char *line;
    char **args;
    char *input_file, *output_file;
    int status = 1;

    do {
        printf("> ");
        line = read_line();

        // Check for pipeline
        if (strchr(line, '|') != NULL) {
            char ***commands = NULL;
            int num_commands = parse_pipeline(line, &commands);

            if (num_commands > 0) {
                status = execute_pipeline(commands, num_commands);
            }

            // Free memory
            for (int i = 0; i < num_commands; i++) {
                free(commands[i]);
            }
            free(commands);
        } else {
            // No pipeline, just a single command
            input_file = NULL;
            output_file = NULL;
            args = parse_command(line, &input_file, &output_file);

            if (args[0] != NULL) {
                status = execute_command(args, input_file, output_file);
            }

            free(args);
        }

        free(line);
    } while (status);
}

int main(int argc, char **argv) {
    // Run command loop
    shell_loop();

    return EXIT_SUCCESS;
}