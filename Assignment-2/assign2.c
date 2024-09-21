#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include<signal.h>

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
void sigint_handler(int sig){
    if(child_pid>0){
        kill(child_pid,SIGINT);
        printf("Terminated the Process with PID %d\n",child_pid);
        child_pid = -1;
    }else{
        printf("Caught Ctrl+c | Enter 'exit' to exit");
    }
    fflush(stdout);
}


int main() {
    char input[max_line];
    char *arr[(max_line / 2) + 1];
    pid_t pid;

    int status;
    bool bkg;

    signal(SIGINT,sigint_handler);

    while (1) {
        printf("Vidus-Rahul's Shell$ ");




            add_to_past_com(input);
            input[strcspn(input, "\n")] = '\0';

            if (strcmp(input, "exit") == 0) {
                break;
            }
            // if (strcmp(arr[0], "^[[A") == 0) {
            //     strcpy(input,com_list[a_count-1]);
            // }

            bkg = backgrd_and_filter(input, arr);

            pid = fork();
            if (strcmp(arr[0], "exit") == 0) {
                break;
            }

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
