#include <stdio.h>

/*For directory related functions (DIRectory ENTries)*/
#include <dirent.h>	

/*stat function includes*/
/*See: http://linux.die.net/man/2/stat*/
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

/*include file struct*/
#include "RFile.h"

#define MAX_LENGTH 200

/*Sorts text file alphabetically*/
void sortTextFile(char *fileName){
	FILE *infile = NULL;
	FILE *outfile = NULL;
	
	char *resultFile = "results.txt";
	char tempData[MAX_LENGTH];
 	char **strData = NULL;
	int i, j, nLines = 0;
	
	if ((infile = fopen(fileName, "r")) != NULL && (outfile = fopen(resultFile, "w+")) != NULL){

		//Read and store in a string list
		while (fgets (tempData, MAX_LENGTH, infile) != NULL){
			if (strchr(tempData, '\n'))
				tempData[strlen(tempData)-1] = '\0';
			strData = (char**)realloc(strData, sizeof(char**)*(nLines+1));
			strData[nLines] = (char*)calloc(MAX_LENGTH,sizeof(char));
 			strcpy(strData[nLines], tempData);
        	nLines++;
		}	

		//Sort the array
    	for(i= 0; i < (nLines - 1); ++i) {
        	for(j = 0; j < ( nLines - i - 1); ++j) {
        	    if(strcmp(strData[j], strData[j+1]) > 0) {
        	        strcpy(tempData, strData[j]);
        	        strcpy(strData[j], strData[j+1]);
        	        strcpy(strData[j+1], tempData);
        	    }
        	}
    	}	

		//Write to outfile
    	for(i = 0; i < nLines; i++)
        	fprintf(outfile,"%s\n",strData[i]);

    	//Free each string
    	for(i = 0; i < nLines; i++)
        	free(strData[i]);

    	//Free string list.
    	free(strData);
		
    	fclose(infile);
    	fclose(outfile);
		
		//Deletes infile
		remove(fileName);
		
		//Changes the name of the outfile
		rename(resultFile, fileName);
	
	}
	
	else{
		perror("Erro ao abrir ficheiro\n");
		exit(-1);
	}
	
	exit(0);
}

/* Unlinks file in path2 and adds a hardlink to path1.
 * 
 * path1 is the path to be written (older file); path2 is the path being overwritten (more recent file)
 * 
 * WORKS!
 * */
void addHardlink(char* path1, char* path2)
{
	FILE *file;
	
	if ((file = fopen("hardLinks.txt", "a")) != NULL){
		if(unlink(path2) != 0)
		{
			perror ("unlink failed");
		}
		
		if (link(path1, path2) != 0)
		{
			perror ("link failed");
		}
		fprintf(file, "%s\n", path2);

		fclose(file);
	}

	else{
		perror("Erro ao abrir ficheiro\n");
		exit(-1);
	}

	exit (0);
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Error: Bad argument usage.\n");	
		printf ("Usage: %s <dir_name>\n", argv[0]);	
		return -1;
	}
	
	pid_t childPid;
	
	childPid = fork();
		
	if(childPid < 0)
	{
		perror("Fork failed.");
		exit(-1);
	}
				
	if(childPid == 0) // child
	{	
		execl("./lsdir", "lsdir", argv[1], (char *)0);
		perror("execl failed");
					
		exit(0);		
	}
				
	if(childPid > 0) // father
	{	
		//continue
		wait(NULL);
		sortTextFile("/tmp/files.txt");
	}
	
	addHardlink("./test/123", "./test/test1/321");
	
	/*When finished, delete /tmp/files.txt
	if(remove("/tmp/files.txt") != 0)	// remove only works if NO PROCESS IS USING THE FILE
	{
		perror ("remove failed");
	}
	*/
	return 0;
}
