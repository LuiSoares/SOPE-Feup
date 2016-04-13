#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

#define MAXLINE 128

int main(void)
{
	int n, fd[2];
	pid_t pid;
	char line[MAXLINE];
	int x, y;
	
	pipe(fd);
	
	pid = fork( );
	
	if (pid > 0) { /* pai */
		
		printf("Write two numbers: ");
		scanf ("%d %d", &x, &y);
		
		char toPass[4];
		
		sprintf(toPass, "%d,%d", x, y);	// pass x and y to string
		
		close(fd[0]); /* fecha lado receptor do pipe */
		write(fd[1], toPass, 4);
				close(fd[1]);
	}
	else { /* filho */
		int soma, sub, mult;
		double div;
		
		close(fd[1]); /* fecha lado emissor do pipe */
		n = read(fd[0], line, MAXLINE);
		
		x = line[0] - '0';
		y = line[2] - '0';
		
		soma = x + y;
		sub = x - y;
		div = (double) x / y;
		mult = x * y;
		
		printf ("%d + %d = %d\n", x, y, soma);
		printf ("%d - %d = %d\n", x, y, sub);
		printf ("%d / %d = %f	\n", x, y, div);
		printf ("%d * %d = %d\n", x, y, mult);
		
		close(fd[0]);
	}
	return 0;
}