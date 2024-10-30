#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>


#define max_line 80
#define NAME "/yoyo"
#define size 4096

char buffer[1000];
char com_list[20][1024];
char time_list[20][1024];
pid_t pid_list[20];
int commands = 0;
volatile int child_pid = -1;

void add_to_past_com(char *string, pid_t pid, long time_taken) {
    strcpy(com_list[commands], string);
    snprintf(time_list[commands], max_line, "%ld ms", time_taken);
    pid_list[commands] = pid;
    commands++;
}

void splitter(char* input, const char* delimiter, char* commands[], int *length) {
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
    
}

void execute_command(char* input) {
    char* commands[10];
    int l = 0;
    splitter(input, " ", commands, &l); // Split commands by spaces
    if (execvp(commands[0], commands) == -1) {
        perror("Failed to execute command!\n");
        exit(1);
    }

}

void signal_handler(int sig) {
    if (sig== SIGINT){
        if (child_pid > 0) {
            kill(child_pid, SIGINT);
            printf("Terminated the Process with PID %d\n", child_pid);
            child_pid = -1;
        }
        else if (child_pid ==-1) {// default implementation
            printf("Caught Ctrl+C | Enter 'exit' to exit\n");
        }
        fflush(stdout);
    }
    else if (sig==SIGCHLD){
        signal(SIGCHLD, SIG_IGN);
    }
}

void set_custom_signals(){
    struct sigaction custom_action; // the struct for use in sigaction function
    struct sigaction default_action;
    
    bzero(&custom_action, sizeof(custom_action)); //setting all the bits to 0 in the memory of action to avoid errors
    bzero(&default_action, sizeof(custom_action));
    
    custom_action.sa_handler= &signal_handler;
    default_action.sa_handler=SIG_DFL;
    
    sigaction(2, &custom_action, NULL); //NULL for no refernce for previous signal handler to be stored
    sigaction(17, &custom_action, NULL);
    //sigaction(?, default_action);
}

int get_time(int pid){
    char cmd[100];
    sprintf(&(cmd[0]), "ps -p %d -o times", pid);
    char output[100];
    
    FILE *fp;
    
    if ((fp = popen(&(cmd[0]), "r")) == NULL) {
        printf("Error!\n");
        return 1;
    }
    
    int i=0;
    int time=0;
    
    while (fgets(&(output[0]), 100, fp) != NULL) {
        i++;
        if (i==2){
            time=atoi(&(output[7]));
        }
        
    }
    
    if (pclose(fp)) {
        printf("Command not found!\n");
        return 1;
    }
    
    return time; //returns time in seconds
}

long max(long a, long b){
    if (a>=b) return a;
    else return b;
}

int main(int argc, char* argv[]) {
    int fd = shm_open(NAME, O_CREAT | O_RDWR, 0666); //o_creat- creating this shm, o_rdwr-
    if (fd==-1){
    perror("Couldn't create a shared memory");
    exit(1);
    }
    
    ftruncate(fd, size); //limiting the size of shm equal size
    
    void* shm_ptr= mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    
    *((int*) shm_ptr)=0; //setting the size of array in shared memory to be 0
    //printf("%d", getpid());
    int ncpu=1;
    int tslice=10;
    if (argc>1){
        ncpu=atoi(argv[1]);
        tslice=atoi(argv[2]);
    }
    char input[1024];
    bool background=false; //to check if the process is to be run in background (& process)
    struct timeval start, end;
    set_custom_signals();
    int once = 0;
    int status;
    bool scheduler=false; //to pause the child process if it uses submit
    int background_proc=0; //to count the number of background process run this far
    int priority=1;
    
    while (true) {
        
        once++;
        
        if (once==1){
            system("clear");
        }
        write(STDOUT_FILENO, "Vidush-Rahul's Shell$ ", 22);
        
        // Input handling
        
        fgets(input, 1024, stdin);
        
        // empty line
        if(input[0] == '\n'){
            continue;
        }
        
        //remove trailing newline char
        input[strcspn(input, "\n")]='\0';
        
        //remove tailing whitespace
        for (int i=strcspn(input, "\0")-1; i--; i>0){
            if (input[i]==' '){
                input[i]='\0';
            }
            else{
                break;
            }
        }
        
        int l=strcspn(input, "\0");
        
        if (input[l-1]=='&'){ //bonus part for background execution
            background=true;
            input[l-1]='\0';
            background_proc++;
        }
        else {
            background=false;
        }
        
        if (strcmp(input, "exit") == 0) {
            for (int i = 0; (i < 20 && i < commands) ; i++) {
                printf("Command: %s | PID: %d | Time: %s\n", com_list[i], pid_list[i], time_list[i]);
            }
            break;
        }
        
        char* arr[10];  // Temporary command parts array for individual parts
        
        splitter(input, " ", arr, &l);  // Split input into command and arguments
        
        // Built-in 'cd' command
        if (strcmp(arr[0], "cd") == 0) {
            if (arr[1] == NULL) {
                ; //do nothing
            }
            else {
                if (chdir(arr[1]) == -1) {
                    perror("cd: no such file or directory: ");
                }
            }
            add_to_past_com(input, getpid(), 0);
            continue;
        }
        
        if(strcmp(arr[0], "history")==0){
            for (int i = 0; i < 20 && i < commands; i++) {
                printf("%s\n", com_list[i]);
            }
            continue;
        }
        
        // Built-in 'clear' command
        if (strcmp(arr[0], "clear") == 0) {
            system("clear");
            continue;
        }
        
        if (strcmp(arr[0], "submit") == 0){
            //no pipes in this by default
            scheduler=true;
            if (48<=input[l-1] && input[l-1]<=57){ //the last character before end is a number
                priority=input[l-1]-48;
            }
            else{
                priority=1;
            }
        }
        else{
            scheduler=false;
        }
        // Record start time
        gettimeofday(&start, NULL);
        
        int pid = fork();
        child_pid = pid;
        if (pid == -1) {
            perror("Shell didn't fork correctly!\n");
        }
        else if (pid == 0) {  // Child process
            
            if (strcmp(arr[0], "submit") == 0){
                //no pipes in this by default
                scheduler=true;
                for (int i=7; i<1024; i++){
                    input[i-7]=input[i];
                    if (input[i]=='\0') break;
                }
                execute_command(input);
            }
            
            // Handling pipes
            int pipes = 0;
            for (int p = 0; input[p] != '\0'; p++) {
                if (input[p] == '|') pipes++;
            }
            
            if (pipes == 0) {  // No pipes, standard execution
                execute_command(input);
            }
            else {  // Handling pipes
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
            exit(0);
        }
        
        else {  // Parent process
            if (scheduler){
                printf("Process: %d submitted for scheduler.\n", pid);
                //waitpid(pid, NULL, 0);
                kill(pid, 19);
                    
                int l=*((int*) shm_ptr);
                *((int*) shm_ptr+l+1)=pid;
                *((int*) shm_ptr+l+2)=priority;
                *((int*) shm_ptr)=l+2;
                
            }
            else if (background ) {
                printf("[%d] %d\n", background_proc, pid);
            }
            else{
                waitpid(pid, &status, 0);
                if (!WIFEXITED(status)) {
                    perror("Error!");
                };  // Wait for child if not a background process
            }
        }
        
        // Record end time and calculate duration
        gettimeofday(&end, NULL);
        
        
        
        //long actual_time_taken=get_time(pid)*1000;
        
        //printf("Actual time");
        long time_taken = (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec) /1000; //time take in milli seconds
        
        // Add to history
        add_to_past_com(input, pid,time_taken);
    }

    munmap(shm_ptr, size);
    close(fd);
    shm_unlink(NAME);
    return 0;
}
