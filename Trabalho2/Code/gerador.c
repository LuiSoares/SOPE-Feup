#include <stdio.h> 
#include <unistd.h> 
#include <pthread.h>
#include <sys/stat.h> 
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>

#define MAX_THREADS 100    // max number of threads

#define FIFO_READ 0
#define FIFO_WRITE 1 

#define STDERR 2 
#define NUMITER 10000

#define DEBUG 1
#define ALLOW_WR_FIFOS 1

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;	// initialization of mutex
int generatingTime, clockUnit;	// main parameters
FILE* geradorLog;	// log file
clock_t ticksOnStart;
int ticks;

/*
 * Struct that stores a vehicle's information.
 * */
typedef struct VehicleInfo
{
	char entryDoor;		// N, S, W or E
	int parkingTime;	// in TICKS
	int vehicleID;		// depending on the counter
	char fifoName[30];		// "vehicle<id>"
	
} VehicleInfo;

/*
 * Prints the information of the vehicle pointed by 'info'.
 * */
void printVehicleInfo (VehicleInfo info)
{
	printf ("%c, %d, %d, %s\n", info.entryDoor, info.parkingTime, info.vehicleID, info.fifoName);
}

/*
 * Generates a pseudo-random number between min_n and max_n.
 * */
int randInRange(int min_n, int max_n)
{
    return rand() % (max_n - min_n + 1) + min_n;
}

/*
 * Creates a random vehicle
 * */
int generateVehicleInfo(VehicleInfo* info, int vehicleID)
{
	int r;
	
	// Choose point of entry with identic probability
	r = randInRange(1, 4);
	switch(r)
	{
		case 1: info->entryDoor = 'N'; break;
		case 2: info->entryDoor = 'S'; break;
		case 3: info->entryDoor = 'E'; break;
		case 4: info->entryDoor = 'W'; break;
		default: printf("Error in random number switch in createVehicle()\n");return -1;
	}
	
	// Parking time with identic probability
	r = randInRange(1, 10); 
	info->parkingTime = r * clockUnit;
	
	// Vehicle ID
	info->vehicleID = vehicleID;
	
	// Vehicle's FIFO name
	sprintf(info->fifoName, "/tmp/vehicle%d", info->vehicleID);
}

/*
 * Function of vehicle's thread
 * */
void *createVehicle (void* inf)
{	
	VehicleInfo* info;
	info = (VehicleInfo*)malloc(sizeof(VehicleInfo));
	info = (VehicleInfo*)inf;
	
	
	// Create private fifo
	if (mkfifo(info->fifoName, 0660) < 0)
	{
		if (errno==EEXIST) 
			printf("FIFO '%s' already exists\n", info->fifoName);
		else 
			printf ("Can't create FIFO -- in createVehicle()\n");
	}
	
	// Write to parque fifo
	int parqueFifoFD;
	char parqueFifoName[] = "/tmp/fifo";
	sprintf(parqueFifoName, "%s%c", parqueFifoName, info->entryDoor);
	
	if(ALLOW_WR_FIFOS)
	{
	if((parqueFifoFD = open(parqueFifoName, O_WRONLY | O_NONBLOCK)) != -1)
	{
		write(parqueFifoFD, info, sizeof(VehicleInfo)); 
		
		close(parqueFifoFD);
	}
	else
	{
		printf ("Error opening fifo '%s' - in createVehicle()\n", parqueFifoName);
	}		
	}
	
	// Receive answer from parque
	int	privateFifoFD;
	char answer[30] = {0};
	
	if(ALLOW_WR_FIFOS)
	{
	if((privateFifoFD = open(info->fifoName, O_RDONLY)) != -1)	// waits for something to be written, when something is written the thread ends
	{
		read(privateFifoFD, answer, sizeof(answer));
	}
	else
	{
		printf ("Error opening fifo '%s' - in createVehicle()\n", info->fifoName);
	}
	}
	
	//--- Begin writing to log file
	pthread_mutex_lock(&mut);
	
	if((geradorLog = fopen("gerador.log", "a+")) == NULL)
		perror("fopen error in createVehicle");
	
	//TODO: ADICIONAR TEMPO DE VIDA
	fprintf (geradorLog, "%8ld; %7d; %6c; %10d; %6s; %s\n", clock() - ticksOnStart, info->vehicleID, info->entryDoor, info->parkingTime, "?", answer);
	
	if(DEBUG)
		printf("I've arrived inside gerador.log's open()\n");
	
	fclose(geradorLog);
	
	free(inf);
		
	pthread_mutex_unlock(&mut);
	//--- End writing to log file
	
	// Delete private fifo
	if (unlink(info->fifoName) < 0)
		printf ("Error destroying '%s'\n", info->fifoName);
		
	
	
	return NULL;
}

int main(int argc, char* argv[]) 
{  
	if (argc != 3)
	{
		 printf("Usage: %s <t_gnr> <u_clk>\n", argv[0]);
		 return 1;
	}
	
	generatingTime = atoi(argv[1]);	// period of time of generation in SECONDS
	clockUnit = atoi(argv[2]);		// clock's unit for generation in TICKS
	
	// Create log file
	geradorLog = fopen("gerador.log", "w"); // w == writing and erases previous contentes if file already existed
	if (geradorLog == NULL)
		printf ("Couldn't create gerador.log file\n");
	else
		fprintf(geradorLog,"t(ticks); id_viat; destin; t_estacion; t_vida; observ\n");
	
	fclose(geradorLog);
	
	
	// i will count the cycles TICKS. timeInterval will tell in how many ticks should a vehicle be created
	int i, timeInterval = 0, vehicleCounter = 10;
	
	VehicleInfo* newVehicle;
	
	srand((unsigned)time(NULL));
	
	ticksOnStart = clock();
	
	for (ticks = 0; ticks < CLOCKS_PER_SEC * generatingTime; ticks++)
	{
		//Defining time interval of vehicle generation
		int r, multiple;
		
		//Thread ID for vehicle -- Thread is DETACHED
		pthread_t tid;
		
		//Create attriubute to generate DETACHED thread
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		
		r = randInRange(1, 10);
		
		if (timeInterval == 0)	// create vehicle and decide another interval
		{
			newVehicle = malloc(sizeof(VehicleInfo));
			vehicleCounter ++;
			generateVehicleInfo(newVehicle, vehicleCounter); 
			
			if(DEBUG)
				printVehicleInfo(*newVehicle);
			
			pthread_create(&tid, &attr, createVehicle, newVehicle);
			
			if (r > 0 <= 5) // 50% probability
				multiple = 0;

			if (r > 5 && r <= 8) // 30% probability
				multiple = 1;
				
			if (r > 8 && r <=10) // 20% probability
				multiple = 2;
				
			timeInterval = multiple * clockUnit;	// set new interval
		}
		
		else
		{
			timeInterval--;	// decrease time
		}
	}
	
	return 0;
}

