#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include  <fcntl.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <string.h>

#include "shared.h"

#define FIFO_READ 0
#define FIFO_WRITE 1

#define DEBUG 1

#define STDERR 2
#define NUMITER 10000

#define NUM_OF_CONTROLLERS 4

#define LOGFILE "parque.log"

char directions[4] = "NSWE";

// initialization of mutex
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutWrite = 	PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mainMut = PTHREAD_MUTEX_INITIALIZER;

int numParkingSpaces, openingTime, freeSpaces, closed;
time_t beginningTime;

//Write to file t(ticks) ; nlug ; id_viat ; observ
int writeToFile(char * message, int id){
	FILE *file;

	file = fopen(LOGFILE, "a+");

	if (file == NULL){
		printf("\nError opening file %s.", LOGFILE);
		return -1;
	}

	time_t actualTime = time(NULL);
	clock_t timeToWrite = (actualTime - beginningTime) * CLOCKS_PER_SEC;

	int spaces = numParkingSpaces - freeSpaces;

	fprintf(file, "%9ld; %4d ; %10d ; %7s\n", timeToWrite, spaces, id, message);

	fclose(file);
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
	vehicle_info_t info = *(vehicle_info_t *) arg;

	char * message = (char *) malloc(100);
	int admission;

	char fifoName[30] = "/tmp/vehicle";
	//sprintf(fifoName, "%s%c", fifoName, info.vehicleID);
	strcpy(fifoName, info.fifoName);

	//Opens fifo to write
	int fd = open(fifoName, O_WRONLY);

	if (fd == -1){
		perror("Opening file Vehicle");
		exit(-1);
	}

	if(DEBUG)
		printf("numlugares: %d\n", freeSpaces);

	if (freeSpaces == 0){
		strcpy(message, PARQUE_CHEIO);
		admission = 0;
	}
	else{
		occupyParkingSpace(); //reserva vaga
		strcpy(message, PARQUE_ENTRADA);
		admission = 1;
	}

	//sends message to vehicle
	write(fd, message, 1000);
	//writes in file .log
	WriteInFileLog(message, info.vehicleID);

	//if vehicle can enter the parking lot
	if (admission == 1){
		if (DEBUG){
			printf("O carro %d entrou no parque", info.vehicleID);
			printf("Tempo de estacionamento: %d\n", info.parkingTime);
		}

		//usleep(ticksToUSeconds(info.parkingTime));
		sleep(info.parkingTime/CLOCKS_PER_SEC);
		freeParkingSpace();
		//when the time runs out
		strcpy(message, PARQUE_SAIDA);
		write (fd, message, sizeof(message));

		//CONFIRMAR
		WriteInFileLog(message, info.vehicleID);
	}
	close(fd);
	remove(fifoName);
	pthread_exit(0);
}

/*
*	Creates controller
*/
void *controller (void *arg){

	if(DEBUG)
		printf("chegou ao controller\n");

	char direction = * (char *) arg;

	char fifoName[30] = "/tmp/fifo";
	sprintf(fifoName, "%s%s", fifoName, &direction);

	if (DEBUG)
		printf("fifoName: %s\n", fifoName);

	struct stat st;

	// If fifo does not exist, create one
	if (stat(fifoName, &st) != 0){
		//If an error occurs when creating fifo
   		if (mkfifo(fifoName, 0666) == -1)
				printf("\nError creating fifo.\n");
	}

	if(DEBUG)
		printf("criou fifo\n");

	//Opens fifo to read
	int fd = open(fifoName, O_RDONLY);

	if (fd == -1){
		perror("Opening file Controller");
		close(fd);
		pthread_exit(NULL);
	}

	vehicle_info_t vehicle;

	while(closed == 0){

		if((read(fd, &vehicle, 1000)) > 0) {

			//Creates detached thread "Valet"
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
			pthread_t val;
			pthread_create(&val, &attr, valet, (void *)&vehicle);
			//pthread_attr_destroy(&attr);
			if (DEBUG)
				printf("criou veiculo %d no Parque %s\n", vehicle.vehicleID, fifoName);
		}
	}

	//ler os pedidos pendentes, fechar fifo, e notificar os clientes do encerramento
	if (close(fd) != -1 )
		printf("fechou '%s'",fifoName);
	remove(fifoName);
	pthread_exit(0);

}

void createLogFile()
{
	FILE* geradorLog;
	geradorLog = fopen(LOGFILE, "w"); // w == writing and erases previous contentes if file already existed
	if (geradorLog == NULL)
		printf ("Couldn't create %s file\n", LOGFILE);
	else
		fprintf(geradorLog,"t(ticks) ; nlug ; id_viatura ; observ\n");
	fclose(geradorLog);
}

int main(int argc, char* argv[]) {

	if (argc != 3){
		 printf("Usage: %s <nr_lugares> <t_abertura>\n", argv[0]);
		 return 1;
	}
	

	//Initializes global variables
	numParkingSpaces = abs(atoi(argv[1]));	// number of parking spaces
	openingTime = abs(atoi(argv[2]));		// duration of the parking lot in SECONDS
	
	if(numParkingSpaces < 0 || openingTime < 0)
	{
		printf ("<nr_lugares> e <t_abertura> devem ser valores positivos.\n");
		return 2;
	}
	
	freeSpaces = numParkingSpaces;
	beginningTime = time(NULL); //returns the time in seconds since 1970
	
	//Set park to closed in the beggining
	closed = 0;

	// Create parks log file
	createLogFile();

	//Creates the threads "Controller" of the park
	pthread_t contN, contS, contW, contE;

	pthread_create(&contN, NULL, controller, (void *)&directions[0]); //North
	pthread_create(&contS, NULL, controller, (void *)&directions[1]); //South
	pthread_create(&contW, NULL, controller, (void *)&directions[2]); //West
	pthread_create(&contE, NULL, controller, (void *)&directions[3]); //East

	sleep(openingTime);
	
	pthread_mutex_lock(&mainMut);
	
	closed = 1;	
		
	//Criar viatura

	int i;
	for (i = 0; i < NUM_OF_CONTROLLERS; i++){
		//getNome
		char fifoName[] = "/tmp/fifo";
		char direction = directions[i];
		sprintf(fifoName, "%s%s", fifoName, &direction);
		fifoName[10] = '\0';
	
		char message[30];
		strcpy(message, PARQUE_ENCERRADO);

		int fd = open(fifoName, O_WRONLY);
		if (fd == -1){
			printf("No such file\n");
		}
		write(fd, &message, sizeof(message));
		close(fd);
	}
	
	pthread_join(contN, NULL);
	pthread_join(contS, NULL);
	pthread_join(contW, NULL);
	pthread_join(contE, NULL);
	
	pthread_mutex_unlock(&mainMut);

	pthread_exit(NULL);
}
