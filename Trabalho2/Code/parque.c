#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include  <fcntl.h>
#include <stdlib.h>
#include <math.h>

#define FIFO_READ 0
#define FIFO_WRITE 1

#define STDERR 2
#define NUMITER 10000

#define NUM_OF_CONTROLLERS 4

#define LOGFILE "parque.log"

char directions[4] = "NSWE";

// initialization of mutex
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutWrite = 	PTHREAD_MUTEX_INITIALIZER;


int numParkingSpaces, openingTime, freeSpaces, closed;
time_t beginningTime;

typedef struct{
	char entryDoor;
	clock_t parkingTime;
	int vehicleID;
	char* fifoName;

} vehicleInfo;

/*
*/
int myTime(int tics){
	return (tics/sysconf(_SC_CLK_TCK)*pow(10,6));
}

char * getFifoName (char *direction){
	char fifoName[] = "/tmp/fifo";
	sprintf(fifoName, "%s%s", fifoName, direction);
	return fifoName;
}

//Write to file t(ticks) ; nlug ; id_viat ; observ
int writeToFile(char * message, int id){
	FILE *file;

	file = fopen(LOGFILE, "w");

	if (file == NULL){
		printf("\nError opening file %s.", LOGFILE);
		return -1;
	}

	time_t actualTime = time(NULL);
	clock_t timeToWrite = (actualTime - beginningTime) * CLOCKS_PER_SEC;

	int spaces = numParkingSpaces - freeSpaces;

	fprintf(file, "%d ; %d ; %d ; %s", (int)timeToWrite, spaces, id, message);

	return 0;
}

void WriteInFileLog(char * message, int id){
	pthread_mutex_lock(&mutWrite);
	writeToFile(message, id);
	pthread_mutex_unlock(&mutWrite);
}

/*
* Decrements the number of free spaces
*/
void occupyParkingSpace(){
	pthread_mutex_lock(&mut);
	freeSpaces--;
	pthread_mutex_unlock(&mut);
}

/*
* Increments the number of free spaces
*/
void freeParkingSpace(){
	pthread_mutex_lock(&mut);
	freeSpaces++;
	pthread_mutex_unlock(&mut);
}



/*
*	Creates valet to accompany vehicle to a free parking space or an exit
*/
void * valet (void *arg){
	vehicleInfo info = *(vehicleInfo *) arg;

	char * message = (char *) malloc(100);
	int admission;

	char fifoName[] = "/tmp/vehicle";
	sprintf(fifoName, "%s%c", fifoName, info.vehicleID);

	//Opens fifo to write
	int fd = open(fifoName, O_WRONLY);

	if (fd == -1){
		perror("Opening file");
		exit(-1);
	}

	if (freeSpaces == 0){
		message = "full";
		admission = 0;
	}
	else{
		message = "entry";
		admission = 1;
		occupyParkingSpace();
	}

	//sends message to vehicle
	write(fd, message, sizeof(message));
	//writes in file .log
	WriteInFileLog(message, info.vehicleID);

	//if vehicle can enter the parking lot
	if (admission == 1){
		//CONFIRMAR
		usleep(myTime(info.parkingTime));
		//sleep(info.ParkingTime/CLOCKS_PER_SEC);

		//when the time runs out
		message="exit";
		write (fd, message, sizeof(message));
		freeParkingSpace();
		WriteInFileLog(message, info.vehicleID);
	}

	pthread_exit(0);
}

/*
*	Creates controller
*/
void *controller (void *arg){
	char direction = * (char *) arg;

	time_t end = time(NULL);
	time_t diff = end - beginningTime;
	if (diff == openingTime)
		closed = 1;

	char * fifoName = getFifoName(direction);

	struct stat st;

	// If fifo does not exist, create one
	if (stat(fifoName, &st) != 0){
		//If an error occurs when creating fifo
   		if (mkfifo(fifoName, 0666) == -1)
			printf("\nError creating fifo.\n");
	}

	//Opens fifo to read
	int fd = open(fifoName, O_RDONLY);

	if (fd == -1){
		perror("Opening file");
		exit(-1);
	}

	char dir;
	clock_t timeToStay;
	int id;
	vehicleInfo vehicle;

	while (closed == 0){
		char * line = NULL;
		size_t len = 0;
		while (getline(&line, &len, fdopen(fd, "r")) == -1) {

			//Receives information about a request from a new client
			sscanf(line, "%c %d %d", &dir, (int *)timeToStay, &id);

			vehicle.entryDoor = dir;
			vehicle.parkingTime = timeToStay;
			vehicle.vehicleID = id;
			vehicle.fifoName = fifoName;

			//Creates detached thread "Valet"
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
			pthread_t val;
			pthread_create(&val, &attr, valet, &vehicle);
			//pthread_attr_destroy(&attr);

		}
		end = time(NULL);
		if (diff == openingTime)
			closed = 1;
	}

	//ler os pedidos pendentes, fechar fifo, e notificar os clientes do encerramento
	close(fd);
	remove(fifoName);
	pthread_exit(0);

}

int main(int argc, char* argv[]) {

	if (argc != 3){
		 printf("Usage: %s <nr_pos> <nr_thrs>\n", argv[0]);
		 printf("Usage: parque <nr_pos> <nr_thrs>\n");
		 return 1;
	}

	//Initializes global variables
	numParkingSpaces = abs(atoi(argv[1]));	// number of parking spaces
	openingTime = abs(atoi(argv[2]));		// duration of the parking lot in SECONDS
	freeSpaces = numParkingSpaces;
	closed = 0;
	beginningTime = time(NULL); //returns the time in seconds since 1970...

	pthread_t contN, contS, contW, contE;

	//Creates the threads "Controller" of the park
	pthread_create(&contN, NULL, &controller, &directions[0]); //North
	pthread_create(&contS, NULL, &controller, &directions[1]); //South
	pthread_create(&contW, NULL, &controller, &directions[2]); //West
	pthread_create(&contE, NULL, &controller, &directions[3]); //East

	sleep(openingTime);

	int i;
	for (i = 0; i < NUM_OF_CONTROLLERS; i++){
		char * fifoName = getFifoName(&directions[i]);
		char message[] = "closed";
		int fd = open(fifoName, O_WRONLY);
		write(fd, &message, sizeof(message));
		close(fd);
	}

	pthread_exit(0);
}
