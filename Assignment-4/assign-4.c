#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <elf.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <setjmp.h>

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;

int fd;


typedef struct Memory{
	void * addr;
	struct Memory* next;
	

} Memory;

Memory* init=NULL;
Memory* curr=NULL;

void* base= NULL;
int page_size=4096; //4kb page size

void* map_segment(void* addr){
	Memory* new= (Memory*) malloc(sizeof(Memory));
	
	int pages= (int)addr/4096; //this gives the page number for curr proc
	if (init==NULL){
		//lseek(fd, 0, SEEK_SET);
		base=mmap(0, 4096, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE|MAP_FIXED, fd, 0);
		printf("%p", base);
		init=new;
		curr=new;
		new->addr=base;
		new->next=NULL;
	}
	else{
		//lseek(fd, pages*4096, SEEK_SET);
 		void* addr=mmap(base+pages*4096, 4096, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE|MAP_FIXED, fd, pages*4096);
		new->addr=addr;
		new->next=NULL;
		curr->next=new; 
		curr=new;
	}
	
	
	//memcpy(virtual_memory, file_data+phdr.p_offset, phdr.p_memsz);

}

void signal_handler(int sig, siginfo_t *info, void *ucontext) {
    if (sig==SIGSEGV){
    	void* addr=info->si_addr;
    	printf("Segmentation fault at address 0x%x\n",  addr); //this gives the address of segmentation fault
    	map_segment(addr);
    }
}



void set_custom_signals(){
    struct sigaction custom_action; // the struct for use in sigaction function
    struct sigaction default_action;
    
    bzero(&custom_action, sizeof(custom_action)); //setting all the bits to 0 in the memory of action to avoid errors
    bzero(&default_action, sizeof(custom_action));
    
    custom_action.sa_handler= &signal_handler;
    custom_action.sa_flags= SA_SIGINFO;

    sigaction(SIGSEGV, &custom_action, NULL);
}





/*
 * release memory and other cleanups
 */
void loader_cleanup(int fd) {
  close(fd);
  Memory* curr_=init;
  while (curr_!=NULL){
  	munmap(curr->addr, 4096);
  	curr_=curr_->next;
  	
  	
  }
  //munmap(mmap_pointer, size); //not necessary as when program is done executing it will automatically free allocated space
}

/*
 * Load and run the ELF executable file
 */
 
int main(int argc, char** argv){
    if(argc != 2) {
        printf("Usage: %s <ELF Executable> \n",argv[0]);
        exit(1);
    }
  
  fd = open(argv[1], O_RDONLY);

  if (fd==-1) {
      printf("error");
      exit(1);
  }
  
  
  //printf("%d", fd);
  
  size_t file_size=lseek(fd, 0, SEEK_END);
  
  lseek(fd, 0, SEEK_SET);
  
  char* file_data= (char*) malloc(file_size);
  read(fd, file_data, file_size);
  
  lseek(fd, 0, SEEK_SET);
  Elf32_Ehdr ehdr;
  
  ssize_t sizeof_elf=read(fd, &ehdr, sizeof(ehdr));
  
  if (sizeof_elf != sizeof(ehdr)){
      printf("The size of loaded elf header doesn't match the standard elf header size");
      exit(1);
  }
  
  off_t e_phoff= ehdr.e_phoff;
  size_t e_phentsize=ehdr.e_phentsize;
  uint16_t e_phnum= ehdr.e_phnum;
  
  Elf32_Addr entry_address=ehdr.e_entry;
  lseek(fd, e_phoff, SEEK_SET);
  
  /*
  void* virtual_memory=mmap(0, 4096*10000, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE, 0, 0);
  void* virtual_memory2=mmap(0 , 8192, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE, 0, 0);
  
  printf("%p\n", virtual_memory);
  printf("%p\n", virtual_memory2);
  */

  /*
  for (int ph=0; ph<e_phnum; ph++){
        Elf32_Phdr phdr;
        
        if (read(fd, &phdr, sizeof(phdr)) != sizeof(phdr)){
            printf("The size of loaded program header doesn't match the standard program header size");
            exit(1);
        }
        
        if (phdr.p_type == PT_LOAD){
          Elf32_Addr start_address= phdr.p_vaddr;
          Elf32_Addr end_address= start_address+phdr.p_memsz;
          
          if (start_address<=entry_address && entry_address<=end_address){
              virtual_memory= mmap(NULL, phdr.p_memsz, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE, 0, 0); //allocating the virtual memory some space
              memcpy(virtual_memory, file_data+phdr.p_offset, phdr.p_memsz); //copying the segment into this space
              offset_from__start=entry_address-start_address;
          }
        }
  }
  
  */
  
  
  //printf("%d\n",*entry_address);
  int (*_start)()= (int (*)())(entry_address);

  // 1. Load entire binary content into the memory from the ELF file.
  // 2. Iterate through the PHDR table and find the section of PT_LOAD 
  //    type that contains the address of the entrypoint method in fib.c
  // 3. Allocate memory of the size "p_memsz" using mmap function 
  //    and then copy the segment content
  // 4. Navigate to the entrypoint address into the segment loaded in the memory in above step
  // 5. Typecast the address to that of function pointer matching "_start" method in fib.c.
  // 6. Call the "_start" method and print the value returned from the "_start"
  int result = _start();
  //printf("User _start return value = %d\n",result);
  loader_cleanup(fd);

}
