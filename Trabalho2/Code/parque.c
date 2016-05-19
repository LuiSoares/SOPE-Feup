#include <stdio.h> 
#include <unistd.h> 
#include <pthread.h> 
#include <sys/stat.h> 
#include <time.h>

#define FIFO_READ 0
#define FIFO_WRITE 1 

#define STDERR 2 
#define NUMITER 10000

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;	// initialization of mutex

int nSpaces, openTime;


/* Creates the controller threads.
 * @param position The position of the controller, which can be either 1 == North, 2 == South, 3 == West or 4 == East.*/
int createController(int position)
{
	char direction;
	
	switch(position)
	{
		case 1: direction = 'N'; break;
		case 2: direction = 'S'; break;
		case 3: direction = 'W'; break;
		case 4: direction = 'E'; break;
		default: direction = '0';
	}
	
	if (position <= 0 || position > 4)
	{
		printf("Wrong argument to initialize controller.\n");
		return -1;
	}
	
	char fifoName[] = "/tmp/fifo";
	sprintf(fifoName, "%s%c", fifoName, direction);
	
	struct stat st;
	
	if (stat(fifoName, &st) != 0)	// if fifo does not exist, create
        mkfifo(fifoName, 0666);
   
	unlink(fifoName);	// delete fifo
	
	return 0; 
}

void * thrfunc(void * arg) { 
  int i;
  
  fprintf(stderr, "Starting thread %c\n", *(char *) arg); 
  for (i = 1; i <= NUMITER; i++)
    write(STDERR, arg, 1); 
  return NULL; 
}

int main(int argc, char* argv[]) {
	
	if (argc != 3)
	{
		 printf("Usage: %s <nr_pos> <nr_thrs>\n", argv[0]);
		 return 1;
	}
	
	nSpaces = atoi(argv[1]);	// number of parking spaces
	openTime = atoi(argv[2]);		// duration of the parking lot in SECONDS
	
	createController(2);

  pthread_t ta, tb;
  char a = '1';
  char b = '2';
  
  //pthread_create(&ta, NULL, thrfunc, &a); 
  //pthread_create(&tb, NULL, thrfunc, &b); 
  //pthread_join(ta, NULL); 
  //pthread_join(tb, NULL); 
  

  return 1; 
}

