
#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;

/*
 * release memory and other cleanups
 */
void loader_cleanup() {

  if(fd <0){
    close(fd);
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char** argv) {
  fd = open(argv[1], O_RDONLY);
  
  if (fd==-1) {
      printf("error");
      exit(1);
  }
  
  
  printf("%d", fd);
  
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
  
  void* virtual_memory;
  Elf32_Addr offset_from_start;
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
              virtual_memory= mmap(NULL, phdr.p_memsz, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_PRIVATE, 0, 0);
              offset_from_start=entry_address-start_address;
              //printf("%p", (int*) virtual_mem);
          }
        }
  }
  
  virtual_memory+=offset_from_start;
  
  printf("%p", virtual_memory);
  
  int (*function_pointer)()= (int (*)())virtual_memory;
  
  int result=function_pointer();
  printf("User _start return value= %d\n", result);
  // 1. Load entire binary content into the memory from the ELF file. //
  // 2. Iterate through the PHDR table and find the section of PT_LOAD
  //    type that contains the address of the entrypoint method in fib.c
  // 3. Allocate memory of the size "p_memsz" using mmap function
  //    and then copy the segment content
  // 4. Navigate to the entrypoint address into the segment loaded in the memory in above step
  // 5. Typecast the address to that of function pointer matching "_start" method in fib.c.
  // 6. Call the "_start" method and print the value returned from the "_start"
  //int result = _start();
  //printf("User _start return value = %d\n",result);
}

int main(int argc, char** argv)
{
  if(argc != 2) {
    printf("Usage: %s <ELF Executable> \n",argv[0]);
    exit(1);
  }
  // 1. carry out necessary checks on the input ELF file
  // 2. passing it to the loader for carrying out the loading/execution
  load_and_run_elf(argv);
  // 3. invoke the cleanup routine inside the loader
  //loader_cleanup();
  return 0;
}

