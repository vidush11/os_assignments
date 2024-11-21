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
#include <setjmp.h>

sigjmp_buf point;

void signal_handler(int sig) {
    if (sig==SIGSEGV){
    	printf("Yoyo");
	longjmp(point, 1);
    }
}

void set_custom_signals(){
    struct sigaction custom_action; // the struct for use in sigaction function
    struct sigaction default_action;
    
    bzero(&custom_action, sizeof(custom_action)); //setting all the bits to 0 in the memory of action to avoid errors
    bzero(&default_action, sizeof(custom_action));
    
    custom_action.sa_handler= &signal_handler;
    custom_action.sa_flags= SA_NODEFER;

    sigaction(SIGSEGV, &custom_action, NULL);
}

int main(int argc, const char * argv[]) {
    int* a=0;
    int x;
    set_custom_signals();
    if (setjmp(point)==1){
    printf("%d", *a);
    }
    else{
    printf("The secret number is %d\n",setjmp(point));
    printf("\nYOYO honey\n");
    }
    
    longjmp(point,1);
    if (setjmp(point)==0){;
    printf("%d", *a);
    }
    

    int arr[10];
    for (int i=0; i<10; i++){
    	arr[i]=i;
    }
    
    for (int i=0; i<10; i++){
    	printf("%d\n", i);
    }
    printf("okay");
    return 0;
}
