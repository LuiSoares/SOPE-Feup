#include <stdio.h> 
#include <unistd.h> 
#include <pthread.h>
#include <sys/stat.h> 
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <sys/times.h>

#include "shared.h"

#define LOG_FILE	"gerador.log"

#define DEBUG 1	
#define ALLOW_WR_FIFOS 1

//---GLOBAL VARIABLES

//parameters
static int generatingTime; 
static clock_t clockUnit;

static clock_t ticksOnStart;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t writeMut = PTHREAD_MUTEX_INITIALIZER;

FILE* geradorLog;

//---AUXILIARY FUNCTIONS	

/*
 * Creates a random vehicle
 * */
int generateVehicleInfo(vehicle_info_t* info, int vehicleID)
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
	
	return 0;
}

/*
 * Generates a random multiple (0, 1 or 2) for the interval between vehicle generations
 * */
 int generateRandomMultiple()
 {
	int r = randInRange(1, 10);
	 
	int multiple;
	 
	if (r > 0 <= 5) // 50% probability
		multiple = 0;

	if (r > 5 && r <= 8) // 30% probability
		multiple = 1;
				
	if (r > 8 && r <=10) // 20% probability
		multiple = 2;
		
	return multiple;
 }
 
 /*
  * Takes a random multiple and multiplies it by the clockUnit and makes the program wait for that interval
  * */
 void sleepForRandomInterval()
 {
	 sleepForTicks(generateRandomMultiple() * clockUnit);
	 if(DEBUG)
		printf ("Interval in seconds: %f\n", generateRandomMultiple() * clockUnit * 1.0 / TICKS_PER_SEC);
 }
 
 /*******************************
 * Function of vehicle's thread *
 ********************************/
void *createVehicle (void* inf)
{	
	vehicle_info_t info = *(vehicle_info_t*)inf;
	
	clock_t threadBegin;
	long int lifetime;
	
	threadBegin = times(NULL);
	
	if(DEBUG)
		printVehicleInfo(info);	
	
	// Create vehcile fifo
	if (mkfifo(info.fifoName, 0660) == -1)
	{
		if (errno==EEXIST) 
			printf("FIFO '%s' already existed\n", info.fifoName);
			
		else 
			printf ("Can't create FIFO -- in createVehicle()\n");
			
		unlink(info.fifoName);	
		free(inf);
		pthread_exit(NULL);
	}
	
	// Write to 'parque' fifo
	int entryFifoFD;
	char entryFifoName[] = "/tmp/fifo";
	sprintf(entryFifoName, "%s%c", entryFifoName, info.entryDoor);
	
	if(DEBUG)
		printf("Controller's fifo: '%s'\n\n", entryFifoName);
	
	//---BEGIN OF CRTICAL AREA
	pthread_mutex_lock(&mut);
	
	if(ALLOW_WR_FIFOS)
	{
		entryFifoFD = open(entryFifoName, O_WRONLY | O_NONBLOCK);
		
		if(entryFifoFD == -1)
		{
			// If park is closed, open will return -1
			
			// WRITE ATTEMP TO LOG FILE
			char closed[100];
			sprintf (closed, "%8ld; %7d; %6c; %10d; %6ld; %s\n", 
			times(NULL) - ticksOnStart, info.vehicleID, info.entryDoor, info.parkingTime, times(NULL) - threadBegin, PARQUE_ENCERRADO);
			appendToLog(LOG_FILE, closed);
			
			close(entryFifoFD);
			unlink(info.fifoName);
			free(inf);
			pthread_mutex_unlock(&mut);
			pthread_exit(NULL);
		}	
		else
		{
			if(DEBUG)
				printf("Fifo '%s' opened in O_WRONLY | O_NONBLOCK.\n", entryFifoName);
		}
		
		if((write(entryFifoFD, &info, 1000)) < 0)
		{
			close(entryFifoFD);
			unlink(entryFifoName);
			free(inf);
			pthread_mutex_unlock(&mut);
			pthread_exit(NULL);
		} 
		else
		{
			if (DEBUG)
				printf("Vehicle information has been sent to '%s'.\n", entryFifoName);
		}
		
		close(entryFifoFD);	
	}
	pthread_mutex_unlock(&mut);
	//---END OF CRITICAL AREA
	
	/*//--- Begin writing ENTRY to log file
	char attempt[100];
	
	sprintf (attempt, "%8ld; %7d; %6c; %10d; %6s; %s\n", 
	times(NULL) - ticksOnStart, info.vehicleID, info.entryDoor, info.parkingTime, "?", PARQUE_ENTRADA);
	
	appendToLog(LOG_FILE, attempt);
	//--- End writing ENTRY to log file*/
	
	// Receive answer from parque
	int	privateFifoFD;
	char answer[30] = {0};
	
	if(ALLOW_WR_FIFOS)
	{
		if((privateFifoFD = open(info.fifoName, O_RDONLY)) != -1)	// waits for something to be written, 
																	//when something is written the thread ends
		{
			if(DEBUG)
				printf ("FIFO '%s' opened in O_RDONLY\n", info.fifoName);
				
			if((read(privateFifoFD, &answer, 30)) == -1)
			{
				close(privateFifoFD);
				unlink(info.fifoName);
				free(inf);
				pthread_exit(NULL);
			}
			else
			{
				//--- Begin writing RESULT to log file
				if((geradorLog = fopen(LOG_FILE, "a+")) == NULL)
					perror("fopen error in createVehicle");
				
				fprintf (geradorLog, "%8ld; %7d; %6c; %10d; %6ld; %s\n", 
				times(NULL)- ticksOnStart, info.vehicleID, info.entryDoor, info.parkingTime, times(NULL)- threadBegin, answer);
				
				if(DEBUG)
					printf("I've arrived inside %s's open()\n", LOG_FILE);
				
				fclose(geradorLog);
				//--- End writing RESULT to log file
			}
			
			close(privateFifoFD);
		}
		else
		{
			printf ("Error opening fifo '%s' - in createVehicle()\n", info.fifoName);
			
			close(privateFifoFD);
			unlink(info.fifoName);
			free(inf);
			pthread_exit(NULL);
		}
		
		if(strcmp(PARQUE_ENTRADA, answer) == 0)
		{
			if((privateFifoFD = open(info.fifoName, O_RDONLY)) != -1)
			{
				if((read(privateFifoFD, &answer, 30)) == -1)
				{
					close(privateFifoFD);
					unlink(info.fifoName);
					free(inf);
					pthread_exit(NULL);
				}
				else
				{
					//--- Begin writing RESULT to log file
					if((geradorLog = fopen(LOG_FILE, "a+")) == NULL)
						perror("fopen error in createVehicle");
					
					fprintf (geradorLog, "%8ld; %7d; %6c; %10d; %6ld; %s\n", 
					times(NULL)- ticksOnStart, info.vehicleID, info.entryDoor, info.parkingTime, times(NULL)- threadBegin, answer);
					
					if(DEBUG)
						printf("I've arrived inside %s's open()\n", LOG_FILE);
					
					fclose(geradorLog);
					//--- End writing RESULT to log file
				}
			}
		}
		
		close(privateFifoFD);
	}
	
	
	// Delete private fifo
	if (unlink(info.fifoName) < 0)
		printf ("Error destroying '%s'\n", info.fifoName);
	
	free(inf);
	pthread_exit(NULL);
}
 
 //---MAIN
 
 int main(int argc, char* argv[]) 
{  
	if (argc != 3)
	{
		 printf("Usage: %s <t_gnr> <u_clk>\n", argv[0]);
		 return 1;
	}
	
	// Set parameters
	generatingTime = atoi(argv[1]);		// period of time of generation in SECONDS
	clockUnit = atoi(argv[2]);			// clock's unit for generation in TICKS
	
	// Create log file
	geradorLog = fopen(LOG_FILE, "w"); // w == writing and erases previous contentes if file already existed
	if (geradorLog == NULL)
		printf ("Couldn't create %s file\n", LOG_FILE);
	else
		fprintf(geradorLog,"t(ticks); id_viat; destin; t_estacion; t_vida; observ\n");
	fclose(geradorLog);
	
	// Set seed for randomizing functions
	srand((unsigned)time(NULL));
	
	// Set up generation of vehicles
	double elapsedTime = 0;	
	int vehicleCounter = 0;
	vehicle_info_t* newVehicle;
	
	// Thread ID for vehicle -- Thread is DETACHED
	pthread_t tid;
		
	// Create attriubute to generate DETACHED thread
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	
	// Begin generation of vehicles
	ticksOnStart = times(NULL);	
	
	do 
	{
		vehicleCounter++;
		newVehicle = malloc(sizeof(vehicle_info_t));
		generateVehicleInfo(newVehicle, vehicleCounter);
		
		sleepForRandomInterval();
		
		pthread_create(&tid, &attr, createVehicle, newVehicle);
		
		elapsedTime = ticksToSeconds(times(NULL) - ticksOnStart);
		
		if(DEBUG)
			printf ("Elapsed Time: %f\n", elapsedTime);
	} while(elapsedTime < generatingTime);
	
	if(DEBUG)
		printf("Got out of the main cycle!\n");
	
	pthread_exit(NULL);
	
	return 0;
}	
 
 
 
