#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

//L-list for background processes
typedef struct process {
    pid_t pid;
    char *command;
    struct process *next;
} Process;

Process *head = NULL;

char prompt[PATH_MAX + HOST_NAME_MAX + 100];
pid_t foreground_pid = -1;

void bglist(void);
void exec_bg_command(char **args);

//Displays SSI prompt in format: username@hostname: current_working_directory >
void show_prompt(void){
    char *username = getlogin();

    if (username == NULL){
        username = getenv("USER");
    }

    char hostname[HOST_NAME_MAX + 1];
    char cwd[PATH_MAX];

    gethostname(hostname, sizeof(hostname));
    getcwd(cwd, sizeof(cwd));

    snprintf(prompt, sizeof(prompt), "%s@%s: %s > ", username, hostname, cwd);

    printf("%s", prompt);
}

//Splits input line from user into command arguments
char **input_type(char *input){
    int size = 8;
    int count = 0;

    char **args = malloc(size * sizeof(char *));
    char *token = strtok(input, " \t\n");

    while (token != NULL){
        if (count >= size -1){
            size = size * 2;
            args = realloc(args, size * sizeof(char *));
        }

        args[count] = token;
        count++;
        token = strtok(NULL, " \t\n");
    }

    args[count] = NULL;
    return args;
}

//Implements built-in cd command
void dir_change(char **args){
    char *directory;

    if (args[1] == NULL){
        directory = getenv("HOME");
    } else if (strcmp(args[1], "~") == 0){
        directory = getenv("HOME");
    } else {
        directory = args[1];
    }

    if (chdir(directory) != 0){
        fprintf(stderr, "Error: failed cd\n");
    }
}

//executes foreground command input from user
void exc_command(char **args){
    if (args[0] == NULL){
        return;
    }

    if (strcmp(args[0], "cd") == 0){
        dir_change(args);
        return;
    }

    if (strcmp(args[0], "bglist") == 0){
        bglist();
        return;
    }

    if (strcmp(args[0], "bg") == 0){
        exec_bg_command(args);
        return;
    }

    pid_t pid = fork();

    if (pid < 0){
        perror("Error: fork failed");
        exit(1);
    }
    if (pid == 0){
        signal(SIGINT, SIG_DFL);

        execvp(args[0], args);
        fprintf(stderr, "%s: No such file or directory\n", args[0]);
        exit(1);
    } else {
        foreground_pid = pid;

        while(waitpid(pid, NULL, 0) == -1){
            if (errno != EINTR){
                perror("Error: waitpid failed");
                break;
            }
        }

        foreground_pid = -1;
    }
}

//break backround process inputed into a string for list
char *command_input(char **args, int index){
    int length = 0;

    for (int i = index; args[i] != NULL; i++){
        length += strlen(args[i]) + 1;
    }

    if (length == 0){
        return NULL;
    }

    char *command = malloc(length * sizeof(char));

    if (command == NULL){
        perror("Error: command malloc");
        exit(1);
    }

    command[0] = '\0';

    for (int i = index; args[i] != NULL; i++){
        strcat(command, args[i]);

        if (args[i + 1] != NULL){
            strcat(command, " ");
        }
    }

    return command;
}

//store background process in Llist
void store_process(pid_t pid, char *command){
    Process *new_process = malloc(sizeof(Process));

    if (new_process == NULL){
        perror("Error: process malloc");
        exit(1);
    }

    new_process->pid = pid;
    new_process->command = command;
    new_process->next = head;
    head = new_process;
}

//Prints currently running background processes
void bglist(void){
    Process *cur = head;
    int count = 0;

    while (cur != NULL){
        printf("%d: %s\n", (int)cur->pid, cur->command);
        count++;
        cur = cur->next;
    }

    printf("Total Background jobs: %d\n", count);
}

//checks for background jobs, while continuing to accept input from user prompt
void check_process(void){
    Process *cur = head;
    Process *prev = NULL;

    while (cur != NULL){
        int status;
        pid_t result = waitpid(cur->pid, &status, WNOHANG);

        if (result == 0){
            prev = cur;
            cur = cur->next;
        } else if (result == cur->pid){
            printf("%d: %s has terminated.\n", (int)cur->pid, cur->command);

            Process *done = cur;

            if (prev == NULL){
                head = cur->next;
                cur = head;
            } else{
                prev->next = cur->next;
                cur = cur->next;
            }

            free(done->command);
            free(done);
        } else{
            prev = cur;
            cur = cur->next;
        }
    }
}

//execute background process
void exec_bg_command(char **args){
    if (args[1] == NULL){
        return;
    }

    pid_t pid = fork();

    if (pid < 0){
        perror("Error: fork");
        return;
    }

    if (pid == 0){
        signal(SIGINT, SIG_IGN);

        execvp(args[1], &args[1]);
        fprintf(stderr, "%s: No such file or directory\n", args[1]);
        exit(1);
    } else{
        char *command = command_input(args, 1);
        store_process(pid, command);
    }
}

//Handles Ctrl-C
void handler(int signum){

    if (foreground_pid > 0){
        kill(foreground_pid, SIGINT);
    } else{
        write(STDOUT_FILENO, "\n", 1);
        write(STDOUT_FILENO, prompt, strlen(prompt));
    }
}

int main(void){
    signal(SIGINT, handler);

    char *input = NULL;
    size_t input_size = 0;

    while(1){
        show_prompt();

        errno = 0;
        size_t num_chars = getline(&input, &input_size, stdin);

        if (num_chars == -1){
            if (errno == EINTR){
                clearerr(stdin);
                continue;
            }
            printf("\n");
            break;
        }
        
        check_process();

        char **args = input_type(input);
        exc_command(args);
        free(args);

        check_process();
    }
    
    free(input);
    return 0;
}