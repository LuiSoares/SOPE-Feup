// PROGRAMA p01a.c
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

void sigint_handler(int signo)
{
 printf("In SIGINT handler ...\n");
}
int main(void)
{ 
 struct sigaction new_action, old_action;	// new and old, so that the early state can be saved
 
 new_action.sa_handler = sigint_handler;	// saying that the handler of new_action is the function above
 
 sigaction(SIGINT, NULL, &old_action);		// null is second argument is reference to output struct, third is reference to input
 
 if (sigaction(SIGINT,&new_action, NULL) < 0)
 {
 fprintf(stderr,"Unable to install SIGINT handler\n");
 exit(1);
 }
 
 printf("Sleeping for 30 seconds ...\n");

 sleep(30);
 
 printf("Waking up ...\n");
 exit(0);
} 
