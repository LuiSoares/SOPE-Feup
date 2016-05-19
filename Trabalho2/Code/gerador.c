#include <stdio.h> 
#include <unistd.h> 
#include <pthread.h>
#include <sys/stat.h> 

#define FIFO_READ 0
#define FIFO_WRITE 1 

#define STDERR 2 
#define NUMITER 10000

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;	// initialization of mutex



void * thrfunc(void * arg) { 
  int i;
  
  fprintf(stderr, "Starting thread %c\n", *(char *) arg); 
  for (i = 1; i <= NUMITER; i++)
    write(STDERR, arg, 1); 
  return NULL; 
}

int main() {  
	
  pthread_t ta, tb;
  char a = '1';
  char b = '2';
  
  pthread_create(&ta, NULL, thrfunc, &a); 
  pthread_create(&tb, NULL, thrfunc, &b); 
  pthread_join(ta, NULL); 
  pthread_join(tb, NULL); 
  return 1; 
}

