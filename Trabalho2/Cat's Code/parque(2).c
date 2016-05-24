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

int numParkingSpaces, openingTime, freeSpaces;
clock_t beginningTime;

//Write to file t(ticks) ; nlug ; id_viat ; observ
int writeToFile(char * message, int id){
	FILE *file;

	file = fopen(LOGFILE, "a+");

	if (file == NULL){
		printf("\nError opening file %s.", LOGFILE);
		return -1;
	}

	clock_t actualTime = times(NULL);
	clock_t timeToWrite = (actualTime - beginningTime) * TICKS_PER_SEC;

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
	strcpy(fifoName, info.fifoName);

	//Opens fifo to write
	int fd = open(fifoName, O_WRONLY);

	if (fd == -1){
		perror("Opening file Vehicle");
		close(fd);
		pthread_exit(NULL);
	}
	if(DEBUG)
		printf("Fifo '%s' opened.\n", fifoName);


	if(DEBUG)
		printf("Numlugares vagos: %d\n", freeSpaces);

	if (freeSpaces == 0){
		strcpy(message, PARQUE_CHEIO);
		admission = 0;
	}
	else{
		occupyParkingSpace(); //reserva vaga
		strcpy(message, PARQUE_ENTRADA);
		admission = 1;
	}

	//writes in file .log
	WriteInFileLog(message, info.vehicleID);

	//sends message to vehicle
	write(fd, message, 1000);

	close(fd);

	//if vehicle can enter the parking lot
	if (admission == 1){
		if (DEBUG){
			printf("O carro %d entrou no parque\n", info.vehicleID);
			printf("Tempo de estacionamento: %d\n", info.parkingTime);
		}

		if (DEBUG)
			printf("Tempo: %d", ticksToUSeconds(info.parkingTime));

		usleep(ticksToUSeconds(info.parkingTime));

		freeParkingSpace();
		//when the time runs out
		strcpy(message, PARQUE_SAIDA);

		//CONFIRMAR
		WriteInFileLog(message, info.vehicleID);
		fd = open(fifoName, O_WRONLY);
		write (fd, message, sizeof(message));
	}
	close(fd);
	remove(fifoName);
	pthread_exit(NULL);
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

	while(1){

		if((read(fd, &vehicle, 1000)) > 0) {

			if (vehicle.vehicleID == 0){
				if (DEBUG)
					printf("Acabou, fdp\n");
				break;
			}

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

	//while(freeSpaces < numParkingSpaces){}
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
	beginningTime = times(NULL); //returns the time in seconds since 1970

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

	//Criar viatura

	int i;
	for (i = 0; i < NUM_OF_CONTROLLERS; i++){
		//Initializes vehicle_0
		vehicle_info_t vehicle_0;
		vehicle_0.vehicleID = 0;
		char dir = directions[i];
		vehicle_0.entryDoor = dir;

		//getNome
		char fifoName[] = "/tmp/fifo";
		char direction = directions[i];
		sprintf(fifoName, "%s%s", fifoName, &direction);
		//fifoName[10] = '\0';

		/*char message[30];
		strcpy(message, PARQUE_ENCERRADO);*/

		int fd = open(fifoName, O_WRONLY);
		if (fd == -1){
			printf("No such file\n");
		}
		write(fd, &vehicle_0, 1000);
		close(fd);
	}

	pthread_join(contN, NULL);
	pthread_join(contS, NULL);
	pthread_join(contW, NULL);
	pthread_join(contE, NULL);

	pthread_mutex_unlock(&mainMut);

	pthread_exit(NULL);
}
