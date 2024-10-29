#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>

#define MAX_LINE 80
#define MAX_PROCESSES 20

char buffer[1000];
char com_list[20][1024];
pid_t pid_list[20];
int commands = 0;
volatile int child_pid = -1;

// Struct to store process information
typedef struct Process {
    int priority;
    char command[1024];
    pid_t pid;
    long wait_time;       // in milliseconds
    long running_time;    // in milliseconds
    struct timeval enqueue_time;
    struct timeval start_time;
    struct timeval end_time;
} Process;

// Priority queue to store processes
Process priorityQueue[MAX_PROCESSES];
int queueSize = 0;

void add_to_past_com(char *string, pid_t pid, long wait_time, long running_time) {
    strcpy(com_list[commands], string);
    pid_list[commands] = pid;
    printf("Command: %s | PID: %d | Wait Time: %ld ms | Running Time: %ld ms\n", string, pid, wait_time, running_time);
    commands++;
}

void splitter(char* input, const char* delimiter, char* commands[], int *length) {
    strcpy(buffer, input);
    char * token = strtok(buffer, delimiter);
    int c = 0;
    while (token != NULL) {
        commands[c] = strdup(token);
        token = strtok(NULL, delimiter);
        c++;
    }
    commands[c] = NULL;
    *length = c;
}

void execute_command(char* input) {
    char* commands[10];
    int l = 0;
    splitter(input, " ", commands, &l);
    if (execvp(commands[0], commands) == -1) {
        perror("Failed to execute command!\n");
        exit(1);
    }
}

void signal_handler(int signal) {
    if (signal == SIGINT) {
        if (child_pid > 0) {
            kill(child_pid, SIGINT);
            printf("Terminated the Process with PID %d\n", child_pid);
            child_pid = -1;
        } else if (child_pid == -1) {
            printf("Caught Ctrl+C | Enter 'exit' to exit\n");
        }
        fflush(stdout);
    }
}

// Priority queue functions
void swap(Process *a, Process *b) {
    Process temp = *a;
    *a = *b;
    *b = temp;
}

void enqueue(Process proc) {
    if (queueSize >= MAX_PROCESSES) {
        printf("Queue is full\n");
        return;
    }
    priorityQueue[queueSize] = proc;
    int i = queueSize;
    queueSize++;
    
    // Up-heap operation to maintain min-heap order based on priority
    while (i > 0 && priorityQueue[(i - 1) / 2].priority > priorityQueue[i].priority) {
        swap(&priorityQueue[i], &priorityQueue[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

Process dequeue() {
    if (queueSize <= 0) {
        printf("Queue is empty\n");
        exit(1);
    }
    Process root = priorityQueue[0];
    priorityQueue[0] = priorityQueue[queueSize - 1];
    queueSize--;
    
    // Down-heap operation to maintain min-heap order
    int i = 0;
    while ((2 * i + 1) < queueSize) {
        int smallest = i;
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        
        if (left < queueSize && priorityQueue[left].priority < priorityQueue[smallest].priority) {
            smallest = left;
        }
        if (right < queueSize && priorityQueue[right].priority < priorityQueue[smallest].priority) {
            smallest = right;
        }
        if (smallest != i) {
            swap(&priorityQueue[i], &priorityQueue[smallest]);
            i = smallest;
        } else {
            break;
        }
    }
    return root;
}

void set_custom_signals(){
    struct sigaction custom_action; // the struct for use in sigaction function
    struct sigaction default_action;
    
    bzero(&custom_action, sizeof(custom_action)); //setting all the bits to 0 in the memory of action to avoid errors
    bzero(&default_action, sizeof(custom_action));
    
    custom_action.sa_handler= &signal_handler;
    default_action.sa_handler=SIG_DFL;
    
    sigaction(2, &custom_action, NULL); //NULL for no refernce for previous signal handler to be stored
    //sigaction(?, default_action);
}


long calculate_time_diff(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
}

void process_and_display_exit_stats() {
    while (queueSize > 0) {
        Process proc = dequeue();
        gettimeofday(&proc.start_time, NULL);  // Start time
        child_pid = fork();
        
        if (child_pid == 0) {
            // Child process executes the command
            execlp(proc.command, proc.command, (char *)NULL);
            perror("Command execution failed");
            exit(1);
        } else {
            // Parent process waits for child and records the times
            waitpid(child_pid, NULL, 0);
            gettimeofday(&proc.end_time, NULL);  // End time
            proc.wait_time = calculate_time_diff(proc.enqueue_time, proc.start_time);
            proc.running_time = calculate_time_diff(proc.start_time, proc.end_time);
            add_to_past_com(proc.command, proc.pid, proc.wait_time, proc.running_time);
            child_pid = -1;
        }
    }
}

int main(int argc, char* argv[]) {
    set_custom_signals();

    char input[1024];
    int status;
    int once = 0;
    
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
        fgets(input, 1024, stdin);


        
        if (input[0] == '\n') continue;
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "clear") == 0) {
                once = 0;
                continue;
            }
        
        if (strcmp(input, "exit") == 0) {
            process_and_display_exit_stats();  // Process and display stats for all queued jobs
            break;
        }
        
        char* arr[10];
        int len;
        splitter(input, " ", arr, &len);
        
        if (strcmp(arr[0], "submit") == 0) {
            int priority = (len > 2) ? atoi(arr[2]) : 1;  // Default priority is 1 if not specified
            Process proc;
            proc.priority = priority;
            snprintf(proc.command, sizeof(proc.command), "%s", arr[1]);
            gettimeofday(&proc.enqueue_time, NULL);  // Record enqueue time
            
            proc.pid = fork();
            
            if (proc.pid == 0) {  // Child process
                if (strcmp(arr[1], "submit") == 0) {
                    kill(getpid(), SIGSTOP);  // Pause the process initially
                }
                execute_command(arr[1]);
                exit(0);
            } else {  // Parent process
                enqueue(proc);
                printf("Process %d with priority %d submitted\n", proc.pid, priority);
            }
        } else {
            int pid = fork();
            struct timeval start, end;
            gettimeofday(&start, NULL);  // Record start time
            
            if (pid == 0) {
                execute_command(input);
                exit(0);
            } else {
                waitpid(pid, &status, 0);
                gettimeofday(&end, NULL);  // Record end time
                
                if (!WIFEXITED(status)) {
                    perror("Error!");
                }
                
                long running_time = calculate_time_diff(start, end);
                add_to_past_com(input, pid, 0, running_time);  // Wait time is zero for non-queued commands
            }
        }
    }
    return 0;
}
