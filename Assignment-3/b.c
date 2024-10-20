#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#define NAME "/yoyo"
#define size 4096

int main(){
	int fd=shm_open(NAME, O_RDWR, 0666);
	void* shm_ptr= mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	
	int s=*((int*) shm_ptr);
	
	for (int i=0; i<s; i++){
		printf("%d", *((int*) shm_ptr+1+i));
	}
	
	munmap(shm_ptr, size);
	close(fd);
	
	shm_unlink(NAME);
	
	return 0;
}
