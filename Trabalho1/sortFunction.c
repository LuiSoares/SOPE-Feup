#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAX_LENGTH 200


/*Sorts text file alphabetically*/
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


int main(){
	sortTextFile("test.txt");  //WORKING
}
