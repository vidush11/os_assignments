#include <iostream>
#include <list>
#include <functional>
#include <stdlib.h>
#include <cstring>
#include <pthread.h>
#include <time.h>

typedef struct{
    int l;
    int h;
    std::function<void(int)> lambda;
    
} arg;

typedef struct{
    int lx;
    int ly;
    int hx;
    int hy;

    std::function<void(int, int)> lambda;
    
} arg2;

int min(int a, int b){
    if (a<b) return a;
    else return b;
    
}

void* run(void* a){
    arg* new_arg=(arg*) a;

    int l=new_arg->l;
    int h=new_arg->h;
    std::function<void(int)> lambda= new_arg->lambda;
    
    free(new_arg);
    for (int i=l ;i<h; i++){
        lambda(i);
    }
    
    return NULL;
}

void* run2(void* a){
    arg2* new_arg=(arg2*) a;

    int lx=new_arg->lx;
    int hx=new_arg->hx;
    
    int ly=new_arg->ly;
    int hy=new_arg->hy;
    
    std::function<void(int,int)> lambda= new_arg->lambda;
    
    free(new_arg);
    for (int j=ly ;j<hy; j++){
        for (int i=lx; i<hx; i++){
            lambda(i,j);
        }
    }
    
    return NULL;
}
void parallel_for(int low, int high, std::function<void(int)> &&lambda, int num_threads){
    clock_t start=clock();
    
    pthread_t ids[num_threads];
    int l=(high-low)/num_threads;
    l+= (l*num_threads<high-low) ?1 :0; //ceil
    
    
    for (int i=1; i<num_threads; i++){
        arg* new_arg= (arg*) malloc (sizeof(arg));
        new_arg->l=low+i*l;
        new_arg->h=min(low+(i+1)*l, high);
        new_arg->lambda= lambda;
        pthread_create(ids+i, NULL,  run, new_arg);
        
    }
    
    arg* new_arg= (arg*) malloc (sizeof(arg));
    new_arg->l=low;
    new_arg->h=min(low+l, high);
    new_arg->lambda= lambda;
    run(new_arg);

    
    for (int i=1; i<num_threads; i++){
        int res;
        pthread_join(*(ids+i), NULL);
    }
    
    clock_t end=clock();
    float seconds = (float)(end - start)/CLOCKS_PER_SEC;
    
    printf("Execution time: %f seconds.\n", seconds);
    
}

void parallel_for(int low1, int high1, int low2, int high2, std::function<void(int, int)> &&lambda, int num_threads){
    clock_t start=clock();
    
    pthread_t ids[num_threads];
    int l=(high2-low2)/num_threads;
    
    
    for (int i=1; i<num_threads; i++){
        arg2* new_arg= (arg2*) malloc (sizeof(arg2));
        new_arg->lx=low1;
        new_arg->hx=high1;
        new_arg->ly=low2+i*l;
        new_arg->hy=min(low2+(i+1)*l, high2);
        new_arg->lambda= lambda;
        pthread_create(ids+i, NULL,  run2, new_arg);
        
    }
    
    arg2* new_arg= (arg2*) malloc (sizeof(arg2));
    new_arg->lx=low1;
    new_arg->hx=high1;
    new_arg->ly=low2;
    new_arg->hy=min(low2+l, high2);
    new_arg->lambda= lambda;
    run2(new_arg);
    
    for (int i=1; i<num_threads; i++){
        int res;
        pthread_join(*(ids+i), NULL);
    }
    
    clock_t end= clock();
    float seconds = (float)(end-start)/CLOCKS_PER_SEC;
    printf("Execution time: %fseconds.\n", seconds);
}

int user_main(int argc, char **argv);

/* Demonstration on how to pass lambda as parameter.
 * "&&" means r-value reference. You may read about it online.
 */
void demonstration(std::function<void()> && lambda) {
  lambda();
}

int main(int argc, char **argv) {
  /* 
   * Declaration of a sample C++ lambda function
   * that captures variable 'x' by value and 'y'
   * by reference. Global variables are by default
   * captured by reference and are not to be supplied
   * in the capture list. Only local variables must be 
   * explicity captured if they are used inside lambda.
   */
  int x=5,y=1;
  // Declaring a lambda expression that accepts void type parameter
  auto /*name*/ lambda1 = /*capture list*/[/*by value*/ x, /*by reference*/ &y](void) {
    /* Any changes to 'x' will throw compilation error as x is captured by value */
    y = 5;
    std::cout<<"====== Welcome to Assignment-"<<y<<" of the CSE231(A) ======\n";
    /* you can have any number of statements inside this lambda body */
  };
  // Executing the lambda function
  demonstration(lambda1); // the value of x is still 5, but the value of y is now 5

  int rc = user_main(argc, argv);
 
  auto /*name*/ lambda2 = [/*nothing captured*/]() {
    std::cout<<"====== YOYO HONEY SINGH ======\n";
    /* you can have any number of statements inside this lambda body */
  };
  demonstration(lambda2);
  return rc;
}

#define main user_main


