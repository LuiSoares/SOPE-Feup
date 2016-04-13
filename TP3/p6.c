// PROGRAMA p6.c
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
int main(void)
{
 pid_t pid;
 int i, j;
 printf("I'm process %d. My parent is %d.\n", getpid(),getppid());
 
 for (i=1; i<=3; i++) {
	pid = fork();
	if ( pid < 0) {
		printf("fork error");
		exit(1);
	}
	else if (pid == 0) {
		printf("I'm process %d. My parent is %d. I'm going to work for 1 second ...\n", getpid(),getppid());
		sleep(1); // simulando o trabalho do filho
		printf("I'm process %d. My parent is %d. I finished my work\n", getpid(),getppid());
		exit(0); // a eliminar na alinea c)
	}
 else // simulando o trabalho do pai
	for (j=1; j<=10; j++) {
		sleep(1);
		printf("father working ...\n");
	}
 }
 exit(0);
}

/*	"ps u" no terminal mostra os processos ativos. Os processos marcados por <defunct> são processos que já terminaram, mas
 * ainda têm informações que precisam de ser usadas, ou seja, processos ZOMBIE.*/
 
/* 	Um processo filho pode ser zombie quando já terminou, e está à espera que o processo pai receba o seu estado de saída.*/

/*	Um processo pai pode ser zombie quando já terminou, mas tem que aguardar que os seus processos-filho terminem.*/
