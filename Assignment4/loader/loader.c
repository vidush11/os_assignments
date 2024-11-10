#include "loader.h"
#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define PAGE_SIZE 4096

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;
char *file_data;
int page_fault_count = 0;
int page_allocation_count = 0;
size_t internal_fragmentation = 0;
size_t fragmentation_with_bss = 0;

// Defined a struct to keep track of all allocated memory
struct assigned_memory {
    void *memory;
    struct assigned_memory *next;
};

// Global variable to keep track of all allocated memory
struct assigned_memory *memory = NULL;

// Recursive function to store memory delails to keep track of it and delete after use.
struct assigned_memory *store_memory(struct assigned_memory *memory, void *address) {
    if (memory == NULL) {
        memory = malloc(sizeof(struct assigned_memory));
        if (memory == NULL) {
            perror("malloc failed");
            exit(1);
        }
        memory->memory = address;
        memory->next = NULL;  
        return memory;
    }

    memory->next = store_memory(memory->next, address);
    return memory;
}

// Recursive function to free all allocated memory
void free_memory(struct assigned_memory *memory) {
    if (memory == NULL) {
        return;
    }

    // Recursively free the next nodes
    free_memory(memory->next);

    // Unmap and free current memory
    if (munmap(memory->memory, PAGE_SIZE) == -1) {
        perror("munmap failed");
    }
    free(memory);
}

// Function to cleanup all allocated memory
void loader_cleanup() {
    if (memory != NULL) {
        free_memory(memory);
        memory = NULL;
    }
    if (file_data != NULL) {
        free(file_data);
        file_data = NULL; 
    }
    if (fd != -1) {
        close(fd);
    }
}


int is_valid_address(void* addr, Elf32_Phdr **segment) {   // To  detect if address access fault generated is a segment fault  or a page fault
    for (int i = 0; i < ehdr->e_phnum; i++) {
        void *seg_start = (void*)phdr[i].p_vaddr;
        void *seg_end = (void*)(phdr[i].p_vaddr + phdr[i].p_memsz);
        if (addr >= seg_start && addr < seg_end) {
            *segment = &phdr[i];
            return 1; //  Return true if the fault is in the PT_LOAD segment ---> indicating a page fault
        }    
    }
    return 0; // Return false if the fault is not in the PT_LOAD segment ---> indicating a "real" segment fault
}

void copy_segment_data(void *page_start, char *file_data, Elf32_Phdr *phdr, void *addr) {
    // Calculate the offset within the segment where this page starts
    size_t offset_in_segment = (uintptr_t)page_start - (uintptr_t)phdr->p_vaddr;

    // *Error_Check* : Check if the offset is beyond the segment's memory size (p_memsz) : Exit Early
    if (offset_in_segment >= phdr->p_memsz) {
        printf("Offset in segment exceeds the segment's memory size. No data to copy.\n");
        return;
    }

    // Set the default number of bytes to copy as the page size (4KB)
    size_t bytes_to_copy = PAGE_SIZE;

    if (offset_in_segment + PAGE_SIZE > phdr->p_filesz) { // If the page is beyond the file size, then we need to copy only the initialized part
        bytes_to_copy = (phdr->p_filesz > offset_in_segment) ? phdr->p_filesz - offset_in_segment : 0; 
        //  Bytes_to_copy is adjusted to only copy the remaining initialized data, calculated as p_filesz - offset_in_segment.
        // If the offset is beyond the file size, then no data is copied, initialling the remaining part to 0 as it is .bss.
    
    }

    //Finally, we map the page to the memory, but with no file_descriptor for now.
    void *mapped_page = mmap(page_start, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC,
                             MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0);

    // *Error_Check* : Check if the mmap failed
    if (mapped_page == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }
    memory = store_memory(memory, (void*)mapped_page);

    // Copy the part of segment required by the program if not zero.
    if (bytes_to_copy > 0) {
        // printf("Copying %zu bytes to address %p\n", bytes_to_copy, mapped_page);
        memcpy(mapped_page, file_data + phdr->p_offset + offset_in_segment, bytes_to_copy);
    }

    // Initialize the remaining part of the page to 0 (Either the full page or the remaining part of the page)
    if (bytes_to_copy < PAGE_SIZE) {
        memset((char*)mapped_page + bytes_to_copy, 0, PAGE_SIZE - bytes_to_copy);
    }

    // Count the page allocation
    page_allocation_count++;

    if (!(offset_in_segment + PAGE_SIZE >= phdr->p_filesz && offset_in_segment + PAGE_SIZE < phdr->p_memsz)) { 
        //  Anything initialized beyond p_fiesz is initialized to 0 ,  allocated as .bss to be used further by the program
        // Assumption: In this code we are assunimg that the .bss part is not considered as internal fragmentation and when we skip that part of bss.
        internal_fragmentation += PAGE_SIZE - bytes_to_copy;
    }

    fragmentation_with_bss += PAGE_SIZE - bytes_to_copy;
}




void segfault_handler(int sig, siginfo_t* info, void* context) {
    void* fault_address = info->si_addr;
    Elf32_Phdr *segment = NULL;
    if (is_valid_address(fault_address, &segment)) {
        printf("Page fault at address %p\n", fault_address);
        void *page_start = (void*)((unsigned long)fault_address & ~(0xFFF));
        copy_segment_data(page_start, file_data, segment,fault_address);
        page_fault_count++;
        // printf("Paging done\n");
        // printf("Page fault handled\n");
    } else {
        fprintf(stderr, "Segmentation fault at invalid address: %p\n", fault_address);
        exit(1);
    }
}

void setup_sigaction() {
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = segfault_handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        perror("sigaction failed");
        exit(1);
    }
}

void load_and_run_elf(char** argv) {
    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open failed");
        exit(1);
    }

    size_t file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    file_data = (char*) malloc(file_size);
    if (file_data == NULL) {
        perror("malloc failed");
        exit(1);
    }

    if (read(fd, file_data, file_size) != file_size) {
        perror("read failed");
        exit(1);
    }

    ehdr = (Elf32_Ehdr *)file_data;
    phdr = (Elf32_Phdr *)(file_data + ehdr->e_phoff);

    setup_sigaction();
    // printf("Starting execution\n");

    Elf32_Addr entry_address = ehdr->e_entry;
    printf("Entry address: %p\n", (void*)entry_address);

    int (*_start)() = (int (*)())entry_address;

    int result = _start();
    printf("User _start return value = %d\n", result);

    printf("Page faults: %d\n", page_fault_count);
    printf("Page allocations: %d\n", page_allocation_count);
    printf("Internal fragmentation: %f KB\n", (float)(internal_fragmentation) / 1024.0);
    // printf("Fragmentation with .bss: %f KB\n", (float)(fragmentation_with_bss) / 1024.0);

}
