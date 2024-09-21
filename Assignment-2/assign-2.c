#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>

void splitter(char* input,const char* delimiter, char* commands[], int *length){
    char * token=strtok(input, delimiter);
    
    int c=0;
    
    commands[c]=token;
    
    while (token){
        token=strtok(NULL, delimiter);
        c++;
        commands[c]=token;
    }
    *length=c;
    return;
}
void execute_command(char* input){
    char* commands[10]; //random
    int l;
    splitter(input, " ", commands, &l);
    
    if (execvp(commands[0], commands) == -1){
        perror("Failed to execute command!\n");
        exit(1);
    }
    
}

int main(){
    
    char* input= malloc(1000);
    
    while (true){
        write(STDOUT_FILENO, "MyShell$ ", 9);
        int i=0;
        int c=getchar();
    
        while (c!='\n' && c!=EOF){
        input[i]=c;
            i++;
            c=getchar();
        }
        input[i]='\0';
        
        if (i==0){perror("Exiting shell\n"); break;}
        
        int pid=fork();
        
        if (pid==-1){
            perror("Shell didn't fork correctly!\n");
        }
        else if (pid==0){//child process
            printf("Child created successfully!\n");
            
            int pipes=0; //counting the number of pipe operators in the input command
            
            for (int p=0; p<i; p++){
                if (input[p]=='|'){
                    pipes++;
                }
            }
                
            if (pipes==0){ //standard case
                execute_command(input);
            }
            else { //let's run commands separated by pipes
                char* parts[10]; //random 10
                int l;
                splitter(input, "|", parts, &l);
                
                int* pipes[9]; //9 pipes for 10 random parts
                
                for (int i=0; i<l-1 ;i++){
                    pipes[i]= malloc(16);
                    if (pipe(pipes[i])==-1){
                        perror("Error during creating pipes");
                        exit(1);
                    }
//                    else {
//                        printf("%d, %d\n", pipes[i][0], pipes[i][1]);
//                    }
                }
                
                for (int i=0; i<l; i++){
                    printf("%s\n", parts[i]);
                }
                for (int i=0; i<l; i++){
                    int status=fork();
                    if (status==0){
                        printf("doing work sir!\n");
                        if (i==0){ //not changing first part's standard input
                            dup2(pipes[i][1], STDOUT_FILENO);
                            close(pipes[i][0]); //closing the reading end
                            execute_command(parts[i]);
                            close(pipes[i][1]);
                        }
                        
                        else if (i==l-1){
                            dup2(pipes[l-2][0], STDIN_FILENO);
                            close(pipes[l-2][1]);
                            execute_command(parts[i]);
                            close(pipes[l-2][0]);
                        }
                        
                        else {
                            dup2(pipes[i-1][0], STDIN_FILENO);
                            dup2(pipes[i][1], STDOUT_FILENO);
                            close(pipes[i-1][1]);
                            close(pipes[i][0]);
                            execute_command(parts[i]);
                            close(pipes[i-1][0]);
                            close(pipes[i][1]);
                            
                        }
                    }
                    else if (status==-1){
                        perror("Failed to fork during executing individual pipe commands");
                        exit(1);
                    }
                    else {
                        waitpid(status, NULL, 0);
                    }
                }
                exit(0);
            }
        }
        
        else {
            wait(NULL);
        }
        
    }
    
    
    //scanf(" %[^\n]%*c", input);
    free(input);
    return 0;
    //printf("%s", input);
}

