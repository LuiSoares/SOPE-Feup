#include <stdio.h> 
#include <unistd.h> 
#include <pthread.h>
#include <sys/stat.h> 

#include <time.h>
#include <stdlib.h>

#define FIFO_READ 0
#define FIFO_WRITE 1 

#define STDERR 2 
#define NUMITER 10000

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;	// initialization of mutex
int generatingTime, clockUnit; 

typedef struct VehicleInfo
{
	char entryDoor;		// N, S, W or E
	int parkingTime;	// in TICKS
	int vehicleID;		// depending on the counter
	char* fifoName;		// "vehicle<id>"
	
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
 * Creates a vehicle*/
int createVehicle(VehicleInfo* info, int vehicleID)
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
	sprintf(info->fifoName, "vehicle%d", info->vehicleID);
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
	
	// i will count the cycles TICKS. timeInterval will tell in how many ticks should a vehicle be created
	int i, timeInterval = 0, vehicleCounter = 0;
	VehicleInfo newVehicle;
	
	srand((unsigned)time(NULL));
	
	for (i = 0; i < CLOCKS_PER_SEC * generatingTime; i++)
	{
		//Defining time interval of vehicle generation
		int r, multiple;
		
		r = randInRange(1, 10);
		
		if (timeInterval == 0)	// create vehicle and decide another interval
		{
			vehicleCounter ++;
			createVehicle(&newVehicle, vehicleCounter); 
			printVehicleInfo(newVehicle);
			
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

