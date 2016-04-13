#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

void ze(){
	printf("world!\n");
}

int main(void){
	
	pid_t pid1, pid2;
	
	pid1 = fork();
		
 if(pid1 > 0) {	// processo pai
	 
		 printf ("Hello ");
		
 }
 
	else{
	
		pid2 = fork();
	
		if (pid2>0)	// processo filho-pai
		
			printf("my ");
			
		else //processo filho-filho
		
			printf("friends\n");
		
	}
 
return 0; 
}
