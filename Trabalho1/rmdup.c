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

/* Unlinks file in path2 and adds a hardlink to path1.
 * 
 * path1 is the path to be written (older file); path2 is the path being overwritten (more recent file)
 * 
 * WORKING
 * */
void addHardlink(char* path1, char* path2, char* initialDir)
{
	FILE *file;
	
	char hardPath[PATH_MAX]; 
	sprintf(hardPath, "%s/hlinks.txt", initialDir);  
	
	if ((file = fopen(hardPath, "a")) != NULL){
		
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
	}
}

/*	Removes absolute Path from given string   		WORKING*/
int removeAbsolutePathFromString(char *str1, char*sub1){
	char str[MAX_LENGTH];
	strcpy(str, str1);
	int i=0, j=0;
	
	int spaceCounter = 0;
	
	while (spaceCounter != 3)
	{
		if (str1[i] == ' ')
			spaceCounter++;
		str[j] = str1[i];
		i++;
		j++;
	}
	
	str[j] = '\0';
	
	strcpy(sub1, str);
	return 0;
}

/*	Gets absolute Path (sub1) from given string (str1)		WORKING*/
int getAbsolutePath(char *str1, char *sub1){
	char str[MAX_LENGTH];
	strcpy(str, str1);
	int i=0, j=0;
	int spaceCounter = 0;
	
	while (spaceCounter != 3)
	{
		if (str1[i] == ' ')
			spaceCounter++;
		i++;
	}
	
	while (str1[i] != ' ')
	{
		str[j] = str1[i];
		i++;
		j++;
	}
	
	str[j] = '\0';
	
	strcpy(sub1, str);
	//printf("%s\n", str);
	return 0;
	
}

/*
* Compares the content of two files, given the absolute path   --- 	 WORKING
*
* @Return Returns 1 if content is the same, 0 if is not, and -1 if error
*
*/
int compareContent(char *fileName1, char *fileName2){ 
	FILE *file1 = NULL;
	FILE *file2 = NULL;

	char tempData1[MAX_LENGTH], tempData2[MAX_LENGTH];
	char **strData1 = NULL;
	char **strData2 = NULL;
	int i, nLines1 = 0, nLines2 = 0;
	//printf("%s\n%s\n", fileName1, fileName2);
	
	if ((file1 = fopen(fileName1, "r")) != NULL && (file2 = fopen(fileName2, "r")) != NULL){
			
		//printf("Abriu os dois ficheiros\n");

		/*Reads the file1 and puts each line info in a string list - strData1*/
		while (fgets (tempData1, MAX_LENGTH, file1) != NULL){
			strData1 = (char**)realloc(strData1, sizeof(char**)*(nLines1+1));
			strData1[nLines1] = (char*)calloc(MAX_LENGTH,sizeof(char));
 			strcpy(strData1[nLines1], tempData1);
        	nLines1++;
		}

		/*Reads the file2 and puts each line info in a string list - strData2*/
		while (fgets (tempData2, MAX_LENGTH, file2) != NULL){
			strData2 = (char**)realloc(strData2, sizeof(char**)*(nLines2+1));
			strData2[nLines2] = (char*)calloc(MAX_LENGTH,sizeof(char));
 			strcpy(strData2[nLines2], tempData2);
        	nLines2++;
		}

		/*If number of lines differ, the files content is not the same*/
		if (nLines1 != nLines2){
			return 0;
		}

		/*Checks if the string lists are equal*/
		for (i=0; i<(nLines1-1); i++){
			if (strcmp(strData1[i], strData2[i]) != 0){ //if strings are different
				return 0;
			}
		}
		
		/*Free each string */
    	for(i = 0; i < nLines1; i++)
        	free(strData1[i]);

    	for(i = 0; i < nLines2; i++)
        	free(strData2[i]);

		free(strData1);
		free(strData2);

		return 1;
	}
	
	else{
		//perror("Erro ao abrir ficheiro\n");
		return -1;
	}
	
}

/*Checks if the are any duplicates in the .txt file */
int isDuplicated(char *fileName, char* initialDir){

	FILE *file1 = fopen(fileName, "r");
	FILE *file2 = fopen(fileName, "r");


	char str1[MAX_LENGTH], str2[MAX_LENGTH];
	char aux1[MAX_LENGTH], aux2[MAX_LENGTH];
	
	
	if (file1 != NULL && file2 != NULL){

		int f1 = 0, f2 =0;

		while (fgets (str1, MAX_LENGTH, file1) != NULL){
			f1++;
			//printf("%d %d\n", f1, f2);

			//enquanto f1 nao apontar para o mesmo que f2
			while (f1 < f2){
				fgets (str1, MAX_LENGTH, file1);
				f1++;
			}

			//enquanto f2 apontar para uma linha menor que f1
			while (f2 < f1){
				fgets (str2, MAX_LENGTH, file2);
				f2++;
			}
			
			while (fgets(str2, MAX_LENGTH, file2) != NULL){
				f2++;
				strcpy(aux1, str1);
				strcpy(aux2, str2);

				//if initials of the strings are the same
				if (str1[0] == str2[0]){
					
					//Remove absolutePath from string
					char sub1[MAX_LENGTH];
					removeAbsolutePathFromString(str1, sub1);

					char sub2[MAX_LENGTH];
					removeAbsolutePathFromString(str2, sub2);
					
					//Check if two lines (without absolutePath) are the same
					if (strcmp(sub1, sub2) == 0){

						//Get absolutePath from string
						char path1[MAX_LENGTH];
						getAbsolutePath(str1, path1);
						
						char path2[MAX_LENGTH];
						getAbsolutePath(str2, path2);

						//printf("Linhas Iguais. \nabsolutePath:%s\n%s\n", path1, path2);
							
						if (strcmp(path1, path2) != 0)
						{

							// if files are duplicates
							if (compareContent(path1, path2) == 1){
								//printf("Duplicado - igual conteudo\n");	
								addHardlink(path1, path2, initialDir);
							}		   

							//if not
							else if (compareContent(path1, path2) == 0){
								//printf("Nao duplicado - diferente conteudo\n");
							}
						}
					}					
					else{
						//printf("%d %d", f1, f2);
						//printf("Nao duplicado - linhas diferentes\n");
						//printf("%s\n%s\n", str1, str2);
						break;
					}
					//printf("%s%s\n", sub1, sub2);
				}
				//If initials are not the same, it doesn't compare strings
				else{
					break;
				}
					
			}
		}
	}

	fclose(file1);
	fclose(file2);
		
	return 0;	
}

/*Sorts text file in with name/file 'filename' alphabetically */
int sortTextFile(char *fileName){
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
		return -1;
	}
		
	return 0;
}


int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Error: Bad argument usage.\n");	
		printf ("Usage: %s <dir_name>\n", argv[0]);	
		return -1;
	}
	
	DIR *dir;
	
	if ((dir = opendir (argv[1])) == NULL)
	{
		perror("Invalid directory");
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
		wait(NULL); //has to wait for lsdir to create /tmp/files.txt, which will store the file's info
	}
	
	/*Sorts file for easier reading*/
	sortTextFile("files.txt"); 

	/*Tests if files in files.txt are duplicated*/
	isDuplicated("files.txt", argv[1]);
	
	//When finished, delete /tmp/files.txt
	if(remove("files.txt") != 0)	// remove only works if NO PROCESS IS USING THE FILE
	{
		perror ("remove failed");
	}
	
	return 0;
}
