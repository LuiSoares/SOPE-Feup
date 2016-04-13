#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

void ze(){
	printf("world!\n");
}

int main(void)
{
	pid_t pid;
	
	pid = fork();
	
 if(pid > 0) {	// processo pai
	 
	printf("Hello ");
	 
	 
 } else {
	 printf ("world!\n");
 }
 
return 0; 
}
