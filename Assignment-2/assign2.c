#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <signal.h>

#define max_line 80

char com_list[20][max_line];
int a_count = 0;

pid_t child_pid = -1;

void add_to_past_com(char *string) {
    for (int i = 19; i > 0; i--) {
        strcpy(com_list[i], com_list[i - 1]);
    }
    strcpy(com_list[0], string);
    ++a_count;
}

bool backgrd_and_filter(char *input, char **arr) {
    int i = 0;
    char *split[max_line];

    split[0] = strtok(input, " ");
    while (split[i] != NULL) {
        arr[i] = split[i];
        split[++i] = strtok(NULL, " ");
    }

    if (i > 0 && strcmp(arr[i - 1], "&") == 0) {
        arr[i - 1] = NULL;
        return true;
    } else {
        arr[i] = NULL;
        return false;
    }
}

void sigint_handler(int sig) {
    if (child_pid > 0) {
        kill(child_pid, SIGINT);
        printf("Terminated the Process with PID %d\n", child_pid);
        child_pid = -1;
    } else {
        printf("Caught Ctrl+C | Enter 'exit' to exit\n");
    }
    fflush(stdout);
}

int main() {
    char input[max_line];
    char *arr[(max_line / 2) + 1];
    pid_t pid;

    int status;
    bool bkg;

    signal(SIGINT, sigint_handler);

    int once = 0;

    while (1) {
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

        printf("Bat's Shell$ ");

        if (fgets(input, max_line, stdin) != NULL) {
            if (input[0] == '\n') {
                continue;
            }
            add_to_past_com(input);
            input[strcspn(input, "\n")] = '\0';

            if (strcmp(input, "exit") == 0) {
                break;
            }

            // Parsing input into command and arguments
            bkg = backgrd_and_filter(input, arr);

            // Handle 'cd' command without forking
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
            if (strcmp(arr[0], "clear") == 0) {
                once = 0;
            }



            pid = fork();
            if (pid < 0) {
                perror("Forking failed | Process creation failed");
                exit(1);
            } else if (pid == 0) {
                child_pid = pid;
                if (execvp(arr[0], arr) == -1) {
                    perror("Command execution failed");
                }
                exit(EXIT_FAILURE);
            } else {
                if (!bkg) {
                    waitpid(pid, &status, 0);
                    child_pid = -1;

                    if (!WIFEXITED(status)) {
                        perror("Error!");
                    }
                } else {
                    printf("Process running in background with PID: %d\n", pid);
                }
            }
        }
    }

    return 0;
}
