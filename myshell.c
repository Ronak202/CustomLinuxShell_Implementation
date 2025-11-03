#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_LINE 1024
#define MAX_ARGS 100

// ---------- SIGNAL HANDLING ----------
void sigint_handler(int sig) {
    printf("\nShell: Use 'exit' to quit\n$ ");
    fflush(stdout);
}

// ---------- PARSE COMMAND INTO ARGS ----------
void parse_command(char *cmd, char **args) {
    char *token = strtok(cmd, " \t");
    int i = 0;
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " \t");
    }
    args[i] = NULL;
}

// ---------- EXECUTE SINGLE COMMAND ----------
void execute_command(char *cmd) {
    char *args[MAX_ARGS];
    parse_command(cmd, args);

    if (args[0] == NULL) return; // empty command

    // built-in: exit
    if (strcmp(args[0], "exit") == 0) {
        exit(0);
    }

    // built-in: cd
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "Shell: expected argument for cd\n");
        } else {
            if (chdir(args[1]) != 0) {
                perror("cd");
            }
        }
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        // handle output redirection
        for (int i = 0; args[i] != NULL; i++) {
            if (strcmp(args[i], ">") == 0) {
                if (args[i+1] == NULL) {
                    fprintf(stderr, "Shell: no file after >\n");
                    exit(1);
                }
                int fd = open(args[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("open");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
                args[i] = NULL; // cut off ">" and filename
                break;
            }
        }
        if (execvp(args[0], args) < 0) {
            perror("execvp");
            exit(1);
        }
    } else if (pid > 0) {
        wait(NULL);
    } else {
        perror("fork");
    }
}

// ---------- EXECUTE PIPELINE ----------
void execute_pipeline(char *commands[], int num_cmds) {

for (int i = 0; i < num_cmds; i++) {
        if (commands[i] == NULL || strlen(commands[i]) == 0) {
            fprintf(stderr, "Shell: invalid null command near pipe\n");
            return;
        }
    }
int pipefd[2 * (num_cmds - 1)];//evergy pipe ned two fd one to write and other to read

    // create pipes
    for (int i = 0; i < num_cmds - 1; i++) {
        if (pipe(pipefd + i*2) < 0) {
            perror("pipe");
            exit(1);
        }
    }

    int pid;
    for (int i = 0; i < num_cmds; i++) {
        pid = fork();
        if (pid == 0) {
            // input redirection (from previous pipe)
            if (i > 0) {
                dup2(pipefd[(i-1)*2], STDIN_FILENO);
            }
            // output redirection (to next pipe)
            if (i < num_cmds - 1) {
                dup2(pipefd[i*2 + 1], STDOUT_FILENO);
            }
            // close all pipes
            for (int j = 0; j < 2*(num_cmds-1); j++) {
                close(pipefd[j]);
            }
            // run command
            char *args[MAX_ARGS];
            parse_command(commands[i], args);
            if (execvp(args[0], args) < 0) {
                perror("execvp");
                exit(1);
            }
        }
    }
    // close all pipes in parent
    for (int i = 0; i < 2*(num_cmds-1); i++) {
        close(pipefd[i]);
    }
    // wait for all children
    for (int i = 0; i < num_cmds; i++) {
        wait(NULL);
    }
}

// ---------- PROCESS INPUT ----------
void process_input(char *input) {
    char *pipeline_cmds[MAX_ARGS];
    int num_pipes = 0;

    // check for pipes first
    char *token = strtok(input, "|");
    while (token != NULL) {
        pipeline_cmds[num_pipes++] = token;
        token = strtok(NULL, "|");
    }
    pipeline_cmds[num_pipes] = NULL;

    if (num_pipes > 1) {
        execute_pipeline(pipeline_cmds, num_pipes);
        return;
    }

    // sequential execution
    char *seq_cmds[MAX_ARGS];
    int seq_count = 0;
    token = strtok(input, "#");
    while (token != NULL) {
        seq_cmds[seq_count++] = token;
        token = strtok(NULL, "#");
    }
    seq_cmds[seq_count] = NULL;

    if (seq_count > 1) {
        for (int i = 0; i < seq_count; i++) {
            execute_command(seq_cmds[i]);
        }
        return;
    }

    // parallel execution
    char *par_cmds[MAX_ARGS];
    int par_count = 0;
    token = strtok(input, "&");
    while (token != NULL) {
        par_cmds[par_count++] = token;
        token = strtok(NULL, "&");
    }
    par_cmds[par_count] = NULL;

    if (par_count > 1) {
        for (int i = 0; i < par_count; i++) {
            pid_t pid = fork();
            if (pid == 0) {
                execute_command(par_cmds[i]);
                exit(0);
            }
        }
        for (int i = 0; i < par_count; i++) {
            wait(NULL);
        }
        return;
    }

    // single command
    execute_command(input);
}

// ---------- MAIN LOOP ----------
int main() {
    char input[MAX_LINE];

    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigint_handler);

    while (1) {
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        printf("%s$ ", cwd);
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            break;
        }
        input[strcspn(input, "\n")] = '\0'; // remove newline

        if (strlen(input) == 0) continue;

        process_input(input);
    }
    return 0;
}