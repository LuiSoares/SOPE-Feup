// P
lude <signal.h>
#include <unistd.h>
#include <stdlib.h>

void sigint_handler(int signo)
{
 printf("In SIGINT handler ...\n");
}
int main(void)
{
 pid_t pid = getpid();
 
 if (signal(SIGINT,sigint_handler) < 0)
 {
 fprintf(stderr,"Unable to install SIGINT handler\n");
 exit(1);
 }
 
 //kill(pid, SIGUSR1);  // sends SIGUSR1 signal
 
 printf("Sleeping for 30 seconds ...\n");
 sleep(30);
 printf("Waking up ...\n");
 exit(0);
} 

/*The signal(<signal>, <signal_handler>) function, DOES NOT SEND A SIGNAL. It only assigns some signal_handler to its signal.*/
