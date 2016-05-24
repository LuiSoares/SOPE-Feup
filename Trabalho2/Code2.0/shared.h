#ifndef _SHARED_H
#define _SHARED_H

#include <time.h>
#include <math.h>

/*
 * Possible answers from parque
 * */
const char PARQUE_ENTRADA[] = "Entrada";
const char PARQUE_CHEIO[] = "Cheio!";
const char PARQUE_SAIDA[] = "Saida";
const char PARQUE_ENCERRADO[] = "Encerrado!";
const char PARQUE_FECHOU[] = "Fechou!";

#define TICKS_PER_SEC sysconf(_SC_CLK_TCK)

/*
 * Struct that stores a vehicle's information.
 * */
typedef struct vehicle_info_t
{
	char entryDoor;		// N, S, W or E
	int parkingTime;	// in TICKS
	int vehicleID;		// depending on the counter
	char fifoName[30];		// "vehicle<id>"
	
} vehicle_info_t;

/*
 * Prints the information of the vehicle pointed by 'info'.
 * */
void printVehicleInfo (vehicle_info_t info)
{
	printf("VehicleID: %d\nEntry: %c\nParking Time: %d\nfifoName: %s\n", 
	info.vehicleID, info.entryDoor, info.parkingTime, info.fifoName);
}

/*
 * Generates a pseudo-random number between min_n and max_n.
 * */
int randInRange(int min_n, int max_n)
{
    return rand() % (max_n - min_n + 1) + min_n;
}

/*
 * Program pauses until nTicks have passed
 * */
void sleepForTicks(clock_t nTicks)
 {
	 usleep(ticksToUSeconds(nTicks));
 }
 
 /*
  * Converts ticks to micro seconds
  * */
 int ticksToUSeconds(int ticks){
	return (ticks/TICKS_PER_SEC * pow(10,6));
}

/*
 * Converts ticks to seconds
 * */
 double ticksToSeconds(int ticks)
 {
	 return (ticks/TICKS_PER_SEC);
 }
 
 /*
 * Appends string to log file
 * */
 void appendToLog(char* logPath, char* string)
 {
	FILE* log;
	
	if((log = fopen(logPath, "a+")) == NULL)
		perror("fopen error in createVehicle");
	
	fprintf (log, "%s", string);
	
	fclose(log);
 }

#endif
