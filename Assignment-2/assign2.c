#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <sys/time.h>

#define max_line 80
char buffer[1000];
char com_list[20][max_line];
char time_list[20][max_line];
pid_t pid_list[20];
int a_count = 0;

void add_to_past_com(char *string, pid_t pid, long time_taken) {
    for (int i = 19; i > 0; i--) {
        strcpy(com_list[i], com_list[i - 1]);
        strcpy(time_list[i], time_list[i - 1]);
        pid_list[i] = pid_list[i - 1];
    }
    strcpy(com_list[0], string);
    snprintf(time_list[0], max_line, "%ld ms", time_taken);
    pid_list[0] = pid;
    ++a_count;
}

bool splitter(char* input, const char* delimiter, char* commands[], int *length) {
    strcpy(buffer, input);
    char * token = strtok(buffer, delimiter);
    int c = 0;
    while (token != NULL) {
        commands[c] = strdup(token);  // Ensure memory allocation for the split command
        token = strtok(NULL, delimiter);
        c++;
    }
    commands[c] = NULL;
    *length = c;
    if (c > 0 && strcmp(commands[c - 1], "&") == 0) {
        commands[c - 1] = NULL;  // Remove '&' for background execution
        return true;
    }
    return false;
}

bool execute_command(char* input) {
    char* commands[10];
    int l = 0;
    bool back = splitter(input, " ", commands, &l); // Split commands by spaces
    if (execvp(commands[0], commands) == -1) {
        perror("Failed to execute command!\n");
        exit(1);
    }
    return back;
}

void sigint_handler(int sig) {
    printf("\nCaught Ctrl+C | Enter 'exit' to exit\n");
    fflush(stdout);
}

int main() {
    char* input = malloc(1000);
    bool back;
    struct timeval start, end;
    signal(SIGINT, sigint_handler);
    int once = 0;
    int status;

    while (true) {
        once++;
        if (once == 1) {
            system("clear");
            printf("\n");
            printf("       _==/          i     i          \\==_\n");
            printf("     /XX/            |\\___/|            \\XX\\\n");
            printf("   /XXXX\\            |XXXXX|            /XXXX\\\n");
            printf("  |XXXXXX\\_         _XXXXXXX_         _/XXXXXX|\n");
            printf(" XXXXXXXXXXXxxxxxxxXXXXXXXXXXXxxxxxxxXXXXXXXXXXX\n");
            printf("|XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX|\n");
            printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
            printf("|XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX|\n");
            printf(" XXXXXX/^^^^\"\\XXXXXXXXXXXXXXXXXXXXX/^^^^^\\XXXXXX\n");
            printf("  |XXX|       \\XXX/^^\\XXXXX/^^\\XXX/       |XXX|\n");
            printf("    \\XX\\       \\X/    \\XXX/    \\X/       /XX/\n");
            printf("       \"\\       \"      \\X/      \"       /\"\n");
            printf("\n");
        }

        write(STDOUT_FILENO, "Bat's Shell$ ", 13);

        // Input handling
        fgets(input, 1000, stdin);
        input[strcspn(input, "\n")] = 0;  // Remove trailing newline

        if (strcmp(input, "exit") == 0) {
            for (int i = 0; i < 20 && i < a_count; i++) {
                printf("Command: %s | PID: %d | Time: %s\n", com_list[i], pid_list[i], time_list[i]);
            }
            break;
        }

        char* arr[10];  // Temporary command array
        int l = 0;
        back = splitter(input, " ", arr, &l);  // Split input into command and arguments

        // Built-in 'cd' command
        if (strcmp(arr[0], "cd") == 0) {
            if (arr[1] == NULL) {
                printf("cd: expected argument\n");
            } else {
                if (chdir(arr[1]) != 0) {
                    perror("cd failed");
                }
            }
            continue;
        }

        // Built-in 'clear' command
        if (strcmp(arr[0], "clear") == 0) {
            once = 0;
            continue;
        }

        // Record start time
        gettimeofday(&start, NULL);

        int pid = fork();
        if (pid == -1) {
            perror("Shell didn't fork correctly!\n");
        } else if (pid == 0) {  // Child process
            // Handling pipes
            int pipes = 0;
            for (int p = 0; input[p] != '\0'; p++) {
                if (input[p] == '|') pipes++;
            }

            if (pipes == 0) {  // No pipes, standard execution
                execute_command(input);
            } else {  // Handling pipes
                char* parts[10];
                int no_of_parts;
                int pipefds[9][2];  // Supports up to 9 pipes (random)

                splitter(input, "|", parts, &no_of_parts);

                for (int i = 0; i < no_of_parts - 1; i++) {
                    if (pipe(pipefds[i]) == -1) {
                        perror("Pipe failed");
                    }
                }

                for (int i = 0; i < no_of_parts; i++) {
                    int pid = fork();
                    if (pid == -1) {
                        perror("Fork failed");
                    } else if (pid == 0) {  // Child process in piping
                        if (i != 0) {  // Redirect stdin for all except first
                            dup2(pipefds[i - 1][0], STDIN_FILENO);
                        }
                        if (i != no_of_parts - 1) {  // Redirect stdout for all except last
                            dup2(pipefds[i][1], STDOUT_FILENO);
                        }

                        // Close all pipe file descriptors
                        for (int j = 0; j < no_of_parts - 1; j++) {
                            close(pipefds[j][0]);
                            close(pipefds[j][1]);
                        }

                        execute_command(parts[i]);
                    }
                }

                // Close pipes in the parent process
                for (int i = 0; i < no_of_parts - 1; i++) {
                    close(pipefds[i][0]);
                    close(pipefds[i][1]);
                }

                // Wait for all child processes to finish
                for (int i = 0; i < no_of_parts; i++) {
                    wait(NULL);
                }
            }
        } else {  // Parent process
            if (!back) {
                waitpid(pid, &status, 0);
                if (!WIFEXITED(status)) {
                        perror("Error!");
                };  // Wait for child if not a background process
            }else{
                printf("Process running in background with PID: %d\n", pid);
            }
        }

        // Record end time and calculate duration
        gettimeofday(&end, NULL);
        long time_taken = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;

        // Add to history
        add_to_past_com(input, pid, time_taken);
    }

    free(input);
    return 0;
}
