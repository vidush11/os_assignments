#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>

int main(){
	
	char* input= malloc(1024);
	
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
		
		if (i==0){perror("Exiting shell"); break;}
		printf("%s\n", input);
	}
	//printf("%d", i);
	
	//scanf(" %[^\n]%*c", input);
	
	//printf("%s", input);
}
