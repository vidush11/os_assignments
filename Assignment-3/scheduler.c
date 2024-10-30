#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#define NAME "/yoyo"
#define max_size 4096

int max (int a, int b){
    if (a>=b) return a;
    else return b;
}

int min(int a, int b){
    if (a<=b) return a;
    else return b;
}
typedef struct Process{ //typedef to avoid writing struct again and again!
    int pid;
    int priority;
    int scheduling_time;
    
} Process;


typedef struct Pr_Queue{
    Process* processes[100]; //arbitarily 100 processes can be stored
    
    int size;
    
} Pr_Queue;

void swap(Pr_Queue* pq, int a, int b){
    Process* temp=pq->processes[a];
    pq->processes[a]=pq->processes[b];
    pq->processes[b]=temp;
    
}
void heapify_up(Pr_Queue* pq, int n){

    while ((n+1)/2-1>=0){
        int x=(n+1)/2-1;
        
        if (pq->processes[x]->priority<pq->processes[n]->priority){
//            printf("swapping");
            swap(pq, x, n);
            n=x;
        }
        else{
            break;
        }
    }
}

void heapify_down(Pr_Queue* pq, int n){
    while (2*n+1<=pq->size-1){
        int x=2*n+1;
        int chosen=0;
        int pri=pq->processes[n]->priority;
        int sc=pq->processes[n]->scheduling_time;
        
        int pri1= pq->processes[x]->priority;
        int sc1=pq->processes[x]->scheduling_time;
        
        if (2*n+2<=pq->size-1){ //have to check both the children
            int pri2=pq->processes[x+1]->priority;
            int sc2=pq->processes[x+1]->scheduling_time;
            
            if (pri2>pri1){
                pri1=pri2;
                sc1=sc2;
                chosen=1;
            }
            else if (pri2==pri1){
                if (sc2<sc1){
                    sc1=sc2;
                    chosen=1;
                }
            }
            
        }
        
        if (pri1>pri || (pri==pri1 && sc>sc1)){
            swap(pq, n,x+chosen);
        }
        else{
            break;
        }
    }
}

void enqueue(Pr_Queue* pq, Process* p){
    pq->processes[pq->size]=p;
    heapify_up(pq, pq->size);
//    printf("%d", pq->processes[0]->pid);
    pq->size++;
}

Process* dequeue(Pr_Queue* pq){ //max element for a max heap
    Process* temp= pq->processes[0];
    
    pq->processes[0]=pq->processes[pq->size-1];
    pq->processes[pq->size-1]=NULL;
    pq->size--;
    heapify_down(pq, 0);
    return temp;
}



int main(int argc, const char * argv[]) {
    
    int ncpu=1;
    int tslice=10;
    if (argc>1){
        ncpu=atoi(argv[1]);
        tslice=atoi(argv[2]);
    }
    
    int fd=shm_open(NAME, O_RDWR, 0666);
    void* shm_ptr= mmap(0, max_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    
    int old_size=0;
    
    Pr_Queue my_queue;
    my_queue.size=0;
    
    Process* to_be_added[ncpu];
    int to_add=0;
    
    while (true){
        for (int i=0; i<to_add; i++){
            Process* proc= to_be_added[i];
            if (kill(proc->pid, 0) == 0){ //process still active
                kill(proc->pid, 19);
                enqueue(&my_queue, proc);
            }
        }
        
        to_add=0;
        
        int s=*((int*) shm_ptr);
        
        if (s>old_size){
            
            for (int i=old_size; i<s; i+=2){
                Process* proc = (Process*) malloc(sizeof(Process));
                proc->pid=*((int*) shm_ptr+i+1);
                proc->priority=*((int*) shm_ptr+i+2);
                proc->scheduling_time=my_queue.size;
                
                enqueue(&my_queue, proc);
            }
            
            old_size=s;
        }
        
        
        
        int pq_size=my_queue.size;
        
        for (int i=0; i<min(pq_size, ncpu); i++){
            Process* proc= dequeue(&my_queue);
            Process* new_process= (Process*) malloc(sizeof(Process));
            //printf("%d\n", proc.pid);
            kill(proc->pid, 18);
            new_process->pid=proc->pid;
            new_process->priority=proc->priority;
            new_process->scheduling_time=max(pq_size, ncpu)-ncpu+i;
            to_be_added[i]=new_process;
            to_add++;
        }
        
        
        usleep(tslice*1000);
        
    }
    
    //munmap(shm_ptr, max_size);
    //close(fd);
    
    //shm_unlink(NAME);
    
    return 0;
}
