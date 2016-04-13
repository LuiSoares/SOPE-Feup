// PROGRAMA p6a.c
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define BUF_LENGTH 256

void exitFunction()
{
 printf ("This program is terminating.\n");
}

int main(int argc, char* argv[])
{

 atexit(exitFunction);
 abort();

 FILE *src, *dst;

 char buf[BUF_LENGTH];
 if(argc <=1)
{
 perror("usage: nome_do_executável file1 file2 \n");
 exit(3);
}
 if ( ( src = fopen( argv[1], "r" ) ) == NULL )
 {
 perror ("Não foi encontrado o ficheiro de input.\n");
 printf ("%s\n", strerror(errno));
 exit(1);
 }
 if ( ( dst = fopen( argv[2], "w" ) ) == NULL )
 {
 perror ("Não foi possível escrever no ficheiro de output.\n");
 exit(2);
 } 

while( ( fgets( buf, BUF_LENGTH, src ) ) != NULL )
 {
 fputs( buf, dst );
 }
 fclose( src );
 fclose( dst );
 exit(0); // zero é geralmente indicativo de "sucesso"
} 
