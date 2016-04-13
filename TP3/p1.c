// PROGRAMA p1.c
#include <sys/types.h>
#include <unistd.h>

int global=1;
int main(void) 
{
int local = 2;
if(fork() > 0) {	// o processo filho tem valor de retorno de fork() == 0, pelo que este é o pai
	printf("PID = %d; PPID = %d\n", getpid(), getppid());
	global++;
	local--;
 } 
 else {
	printf("PID = %d; PPID = %d\n", getpid(), getppid());
	global--;
	local++;
 }
 printf("PID = %d - global = %d ; local = %d\n", getpid(), global, local);
 return 0;
} 

/*Variáveis globais ou locais são tratadas da mesma forma, quer haja fork() ou não.*/
