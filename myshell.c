#include <stdio.h>
#include <string.h>
#include <stdlib.h>        // exit()
#include <unistd.h>        // fork(), getpid(), exec()
#include <sys/wait.h>      // wait()
#include <signal.h>        // signal()
#include <fcntl.h>         // close(), open()

// Data structures for shell operations
char *cmd_list[50];           // Storage for multiple command strings
char *token_array[50];        // Array for storing parsed tokens
int total_cmds = 0;           // Counter for parsed commands
int parallel_mode = 0;        // Indicator for concurrent execution (&&)
int sequential_mode = 0;      // Indicator for sequential execution (##)
int redirect_output = 0;      // Indicator for output redirection (>)
char *target_file = NULL;     // File path for output redirection

// Signal interrupt handler function
void interrupt_handler(int signal_num) {
    // Ignore interrupt signals to keep shell alive
    // Shell should continue operation despite Ctrl+C or Ctrl+Z
}

void parseInput(char *user_input) {
    // Parse input string into commands and arguments based on delimiters
    // Supports &&, ##, >, and space-separated tokens
    
    // Initialize parsing state
    total_cmds = 0;
    parallel_mode = 0;
    sequential_mode = 0;
    redirect_output = 0;
    target_file = NULL;
    
    // Clean newline character from input
    char *line_end = strchr(user_input, '\n');
    if (line_end) *line_end = '\0';
    
    // Skip empty input
    if (strlen(user_input) == 0) {
        return;
    }
    
    // First priority: detect output redirection
    char *redirect_symbol = strstr(user_input, ">");
    if (redirect_symbol) {
        redirect_output = 1;
        *redirect_symbol = '\0';  // Break string at redirection symbol
        
        // Extract target filename after '>' symbol
        char *file_part = redirect_symbol + 1;
        while (*file_part == ' ') file_part++;  // Skip whitespace
        target_file = strsep(&file_part, " \t\n");
    }
    
    // Second priority: detect parallel execution pattern
    char *and_symbol = strstr(user_input, "&&");
    if (and_symbol && !redirect_output) {
        parallel_mode = 1;
        char *input_duplicate = strdup(user_input);
        char *cmd_token = strsep(&input_duplicate, "&");
        while (cmd_token != NULL) {
            if (strlen(cmd_token) > 0 && cmd_token[0] != '&') {
                // Clean whitespace from command
                while (*cmd_token == ' ') cmd_token++;
                char *cmd_end = cmd_token + strlen(cmd_token) - 1;
                while (cmd_end > cmd_token && *cmd_end == ' ') cmd_end--;
                *(cmd_end + 1) = '\0';
                
                if (strlen(cmd_token) > 0) {
                    cmd_list[total_cmds++] = strdup(cmd_token);
                }
            }
            cmd_token = strsep(&input_duplicate, "&");
        }
        free(input_duplicate);
        return;
    }
    
    // Third priority: detect sequential execution pattern
    char *hash_symbol = strstr(user_input, "##");
    if (hash_symbol && !redirect_output) {
        sequential_mode = 1;
        char *input_duplicate = strdup(user_input);
        char *cmd_token = strsep(&input_duplicate, "#");
        while (cmd_token != NULL) {
            if (strlen(cmd_token) > 0 && cmd_token[0] != '#') {
                // Clean whitespace from command
                while (*cmd_token == ' ') cmd_token++;
                char *cmd_end = cmd_token + strlen(cmd_token) - 1;
                while (cmd_end > cmd_token && *cmd_end == ' ') cmd_end--;
                *(cmd_end + 1) = '\0';
                
                if (strlen(cmd_token) > 0) {
                    cmd_list[total_cmds++] = strdup(cmd_token);
                }
            }
            cmd_token = strsep(&input_duplicate, "#");
        }
        free(input_duplicate);
        return;
    }
    
    // Default: single command with space-separated arguments
    char *input_duplicate = strdup(user_input);
    char *arg_token = strsep(&input_duplicate, " \t");
    int token_idx = 0;
    
    while (arg_token != NULL) {
        if (strlen(arg_token) > 0) {
            token_array[token_idx++] = strdup(arg_token);
        }
        arg_token = strsep(&input_duplicate, " \t");
    }
    token_array[token_idx] = NULL;  // Null terminator for exec family
    
    free(input_duplicate);
}

void executeCommand(char **cmd_tokens) {
    // Fork new process and execute single command using exec system calls
    
    if (cmd_tokens == NULL || cmd_tokens[0] == NULL) {
        return;
    }
    
    // Special handling for change directory command
    if (strcmp(cmd_tokens[0], "cd") == 0) {
        if (cmd_tokens[1] == NULL) {
            // cd without arguments - change to home
            chdir(getenv("HOME"));
        } else if (strcmp(cmd_tokens[1], "..") == 0) {
            // cd .. - navigate to parent directory
            chdir("..");
        } else {
            // cd to specific directory path
            if (chdir(cmd_tokens[1]) != 0) {
                perror("cd");
            }
        }
        return;
    }
    
    // Create child process for external command execution
    pid_t child_pid = fork();
    
    if (child_pid == 0) {
        // Child process execution block
        if (execvp(cmd_tokens[0], cmd_tokens) == -1) {
            printf("Shell: Incorrect command\n");
            exit(1);
        }
    } else if (child_pid > 0) {
        // Parent process - wait for child completion
        int exit_status;
        wait(&exit_status);
    } else {
        // Fork operation failed
        perror("fork");
    }
}

void executeParallelCommands() {
    // Execute multiple commands concurrently using parallel processing
    
    pid_t process_ids[total_cmds];
    int idx;
    
    // Create child processes for all commands simultaneously
    for (idx = 0; idx < total_cmds; idx++) {
        // Convert command string to argument array
        char *cmd_copy = strdup(cmd_list[idx]);
        char *cmd_tokens[50];
        char *word = strsep(&cmd_copy, " \t");
        int token_count = 0;
        
        while (word != NULL) {
            if (strlen(word) > 0) {
                cmd_tokens[token_count++] = word;
            }
            word = strsep(&cmd_copy, " \t");
        }
        cmd_tokens[token_count] = NULL;
        
        // Handle cd command without forking
        if (strcmp(cmd_tokens[0], "cd") == 0) {
            if (cmd_tokens[1] == NULL) {
                chdir(getenv("HOME"));
            } else if (strcmp(cmd_tokens[1], "..") == 0) {
                chdir("..");
            } else {
                if (chdir(cmd_tokens[1]) != 0) {
                    perror("cd");
                }
            }
            process_ids[idx] = -1;  // Mark as internally handled
            free(cmd_copy);
            continue;
        }
        
        // Fork child process for external commands
        process_ids[idx] = fork();
        
        if (process_ids[idx] == 0) {
            // Child process block
            if (execvp(cmd_tokens[0], cmd_tokens) == -1) {
                printf("Shell: Incorrect command\n");
                exit(1);
            }
        } else if (process_ids[idx] < 0) {
            // Fork failure handling
            perror("fork");
        }
        
        free(cmd_copy);
    }
    
    // Wait for all spawned child processes to finish
    for (idx = 0; idx < total_cmds; idx++) {
        if (process_ids[idx] > 0) {
            int exit_status;
            waitpid(process_ids[idx], &exit_status, 0);
        }
    }
}

void executeSequentialCommands() {
    // Execute multiple commands one after another in order
    
    int idx;
    for (idx = 0; idx < total_cmds; idx++) {
        // Convert command string to argument array
        char *cmd_copy = strdup(cmd_list[idx]);
        char *cmd_tokens[50];
        char *word = strsep(&cmd_copy, " \t");
        int token_count = 0;
        
        while (word != NULL) {
            if (strlen(word) > 0) {
                cmd_tokens[token_count++] = word;
            }
            word = strsep(&cmd_copy, " \t");
        }
        cmd_tokens[token_count] = NULL;
        
        // Execute current command and wait for completion
        executeCommand(cmd_tokens);
        
        free(cmd_copy);
    }
}

void executeCommandRedirection() {
    // Execute single command with stdout redirected to specified file
    
    if (token_array[0] == NULL || target_file == NULL) {
        printf("Shell: Incorrect command\n");
        return;
    }
    
    // Change directory commands don't need output redirection
    if (strcmp(token_array[0], "cd") == 0) {
        executeCommand(token_array);
        return;
    }
    
    pid_t child_pid = fork();
    
    if (child_pid == 0) {
        // Child process - setup output redirection
        int file_descriptor = open(target_file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
        if (file_descriptor == -1) {
            perror("open");
            exit(1);
        }
        
        // Redirect standard output to target file
        if (dup2(file_descriptor, STDOUT_FILENO) == -1) {
            perror("dup2");
            close(file_descriptor);
            exit(1);
        }
        
        close(file_descriptor);
        
        // Execute command with redirected output
        if (execvp(token_array[0], token_array) == -1) {
            printf("Shell: Incorrect command\n");
            exit(1);
        }
    } else if (child_pid > 0) {
        // Parent process - wait for child completion
        int exit_status;
        wait(&exit_status);
    } else {
        // Fork operation failed
        perror("fork");
    }
}

int main() {
    // Shell initialization and main execution loop
    char *user_input = NULL;
    size_t buffer_size = 0;
    char working_dir[1024];
    
    // Configure signal handlers for keyboard interrupts
    signal(SIGINT, interrupt_handler);   // Handle Ctrl+C
    signal(SIGTSTP, interrupt_handler);  // Handle Ctrl+Z
    
    while(1) {  // Main shell loop - runs until exit command
        // Display current working directory as prompt
        if (getcwd(working_dir, sizeof(working_dir)) != NULL) {
            printf("%s$", working_dir);
        } else {
            printf("$");
        }
        
        // Read complete input line from user
        ssize_t chars_read = getline(&user_input, &buffer_size, stdin);
        
        if (chars_read == -1) {
            // Handle end-of-file condition (Ctrl+D)
            printf("\n");
            break;
        }
        
        // Process user input through parser
        parseInput(user_input);
        
        // Handle empty input case
        if (total_cmds == 0 && token_array[0] == NULL) {
            continue;
        }
        
        if (token_array[0] != NULL && strcmp(token_array[0], "exit") == 0) {  // Exit command detection
            printf("Exiting shell...\n");
            break;
        }
        
        if (parallel_mode) {
            executeParallelCommands();      // Handle concurrent command execution
        } else if (sequential_mode) {
            executeSequentialCommands();   // Handle sequential command execution
        } else if (redirect_output) {
            executeCommandRedirection();   // Handle output redirection to file
        } else {
            executeCommand(token_array);   // Handle single command execution
        }
        
        // Memory cleanup after command execution
        if (parallel_mode || sequential_mode) {
            for (int i = 0; i < total_cmds; i++) {
                free(cmd_list[i]);
            }
        } else {
            for (int i = 0; token_array[i] != NULL; i++) {
                free(token_array[i]);
                token_array[i] = NULL;
            }
        }
    }
    
    // Final cleanup before exit
    if (user_input) free(user_input);
    return 0;
}