#ifndef RFILE_H
#define RFILE_H

/*for time functions and structs*/
#include <time.h>

/*
 * Struct that'll be used to store regular files' properties. 
 * */
typedef struct 
{
	char* name;			// file's name
	long long size;		// file's size in bytes
	unsigned long mode;	// file permissions in decimal representation
	time_t lastMDate;	// last file modification date in time_t struct
	char* absolutePath;	// file's absolute path in the system
}RFile;

#endif
