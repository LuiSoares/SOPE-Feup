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
	
	struct twoInt{
		int x, y;
	};
	
	struct twoInt t;
	
	if (pid > 0) { /* pai */
		
		printf("Write two numbers: ");
		scanf ("%d %d", &t.x, &t.y);
		
		close(fd[0]); /* fecha lado receptor do pipe */
		write(fd[1], &t, sizeof(struct twoInt));	// passses the reference
		close(fd[1]);
	}	
	else { /* filho */
		int soma, sub, mult;
		double div;
		
		close(fd[1]); /* fecha lado emissor do pipe */
		n = read(fd[0], &t, sizeof(struct twoInt));	// uses the reference
		
		x = t.x;
		y = t.y;
		
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