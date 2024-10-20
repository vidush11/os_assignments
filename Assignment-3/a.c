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
	int fd = shm_open(NAME, O_CREAT | O_RDWR, 0666); //o_creat- creating this shm, o_rdwr- 
	if (fd==-1){
		perror("Couldn't create a shared memory");	
		exit(1);
	}
	
	ftruncate(fd, size); //limiting the size of shm equal size
	
	void* shm_ptr= mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	

	int numbers[]={1,2,3,4};
	
	int s=sizeof(numbers)/sizeof(int);
	*((int*) shm_ptr)=s;
	
	for (int i=0; i<s; i++){
		*((int*) shm_ptr+i+1)=numbers[i];
	}
	
	printf("YOYO honey");
	//(int*) shm_ptr[1]=
	//snprintf((int*) shm_ptr, size, numbers); //limit the size of writing buffer as size so no overflow occurs
	
	munmap(shm_ptr, size);
	close(fd);
	// shm_unlink(NAME);
	return 0;
	
	
}
