#include <stdio.h> 
#include <unistd.h> 
#include <time.h>
#include  <fcntl.h>
//THREAD
#include <pthread.h> 
//FIFOS
#include <sys/stat.h> 
#include <sys/types.h>


#define BUF_SIZE 100

int numParkingSpaces, openingTime;

typedef struct{

	char entryDoor;
	clock_t parkingTime;
	int vehicleID;
	char* fifoName;

} vehicleInfo;


/*
	Creates valet to accompany vehicle to a free parking space or an exit
*/
void * valet (void *arg);

/*
	Creates controller
*/
void *controller (void *direction){

	char fifoname[] = "/tmp/fifo";
	sprintf(fifoName, "%s%c", fifoName, (char *)direction);
	
	struct stat st;
	
	// If fifo does not exist, create one
	if (stat(fifoName, &st) != 0){	
		//If an error occurs when creating fifo
   		if (mkfifo(fifoname, 0666) == -1)
			printf("\nError creating fifo.\n");
	}
	
	//Opens fifo to read and write
	int fd = open(fifoname, O_RDWR); //or O_RDONLY (read only)
	
	if (fd == -1){
		perror("Opening file");
		exit(-1);
	}

	c = getchar();
	size_t size = sizeof(char) * BUF_SIZE;
	char * buffer = malloc(size);
	
	char dir;
	clock_t time;
	int id;
	vehicleInfo vehicle;

	while (c != EOF) {

		//Receives information about a request from a new client

 		getline(&buffer,&size,fdopen(fd, "r"));
		sscanf(buffer, "%c %l %d", &dir, &time, &id);
	
		vehicle.entryDoor = dir;
		vehicle.parkingTime = time;
		vehicle.vehicleID = id;
		vehicle.fifoName = fifoName;

		pthread_t val;

		pthread_create(valet, NULL, valet, vehicle);

		//cria arrumador para passar informacao do veiculo e este ser acompanhado a um lugar ou à saída
    	c = getchar();
  	}


}

int main(int argc, char *argv[]){
	
	//Wrong number of arguments
	if (argc != 3){
		printf("Usage: %s <NumParkingSpaces> <OpeningTime>", argv[0]);
		return -1;
	}

	pthread_t contN, contS, contW, contE;

	//Cria os controladores de cada entrada/saída do parque
	pthread_create(&contN, NULL, controller, 'N'); //North
	pthread_create(&contS, NULL, controller, 'S'); //South
	pthread_create(&contW, NULL, controller, 'W'); //West
	pthread_create(&contE, NULL, controller, 'E'); //East

	return 0;
}
