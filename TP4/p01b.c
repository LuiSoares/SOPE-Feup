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
 pid_t pid = getpid();
 
 unsigned int sleepingInterval = 30;
 
 if (signal(SIGINT,sigint_handler) < 0)
 {
 fprintf(stderr,"Unable to install SIGINT handler\n");
 exit(1);
 }
 
 // ex. a
 //kill(pid, SIGUSR1);  // sends SIGUSR1 signal
 
 printf("Sleeping for 30 seconds ...\n");

 while(sleepingInterval > 0)	// while the interval left is not zero
 { 
	sleepingInterval = sleep(sleepingInterval);
 }
 printf("Waking up ...\n");
 exit(0);
} 

// ex. b
/* The sleep() function makes the program sleep for n seconds or UNITL THE PROGRAM RECEIVES A SIGNAL, WHICH
 * IT DOES NOT IGNORE!*/

/* The sleep() function does return a value. If it was not interrupted, it returns 0. If it has been
 * interrupted by a signal, IT RETURNS THE TIME LEFT FOR THE SLEEP.*/
