// PROGRAMA p2.c
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main(void) {
 //write(STDOUT_FILENO,"1",1);
 printf("1\n");

 if(fork() > 0) {
 	//write(STDOUT_FILENO,"2",1);
 	//write(STDOUT_FILENO,"3",1);
 	printf("23");
 } 
 else {
 	//write(STDOUT_FILENO,"4",1);
 	//write(STDOUT_FILENO,"5",1);
 	printf ("4");
 	printf ("5");
 }
 //write(STDOUT_FILENO,"\n",1);
 printf("\n");
 return 0;
}

/*	write() é uma função do sistema, pelo que envia a informação diretamente ao sistema operativo. Ao executar write(),
 * os caracteres são imediatamente impressos.*/
 
/* 	printf() é uma função de biblioteca. Esta função usa um buffer para economizar as chamadas de sistema, pelo que só imprime
 * quando encontra um '\n' ou quando o buffer está cheio.*/
 
/* 	Quando o primeiro printf() não tem '\n' o 1 fica armazenado no buffer, TANTO para o processo pai COMO para o processo filho, sendo
 * impresso 2 vezes. Quando se adicona o '\n', o buffer é esvaziado, pelo que o '1' só é impresso uma vez.*/
