#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_LENGTH 200

/*Converts duplicated file to hardLink ---  POR FAZER*/
/*int convertToHardLink(char *path1, char*path2){  
	FILE *file;
	int status;
	
	if ((file = fopen("hardLinks.txt", "a")) != NULL){
		status = link(path1, path2);
		fprintf(file, "%d", status);
		//incrementar numero de hardlinks
	}

	else{
		perror("Erro ao abrir ficheiro\n");
		return -1;
	}

	return 0;	
}*/


/*	Removes absolute Path from given string   		WORKING*/
int removeAbsolutePathFromString(char *str1, char*sub1){
	char str[MAX_LENGTH];
	strcpy(str, str1);
	char *a = strstr(str, "/");
	char *b = strrchr (str, ' ');
	if ((a == NULL) || (b == NULL) || (b < a))
    	return -1;

	memmove(a, b, strlen(b)+1);
	//printf("%s\n", str);
	strcpy(sub1, str);
	return 0;
}


/*	Gets absolute Path (sub1) from given string (str1)		WORKING*/
int getAbsolutePath(char *str1, char *sub1){
	int i=0, j=0, num = 0;
	char str[MAX_LENGTH];
	strcpy(str, str1);
	
	while (str[i] != '/'){
		i++;
		j++;
	}
	
	while (str[i] != ' '){
		num++;
		i++;
	}
	
	char sub[num+1];
	memcpy(sub, &str[j], num);
	strcpy(sub1, sub);
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
	int i, j, nLines1 = 0, nLines2 = 0;
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
int isDuplicated(char *fileName){

	FILE *file1 = fopen(fileName, "r");
	FILE *file2 = fopen(fileName, "r");

	int num = 0, i=0, j=0;
	char str1[MAX_LENGTH], str2[MAX_LENGTH],barra[2] = "/";
	char aux1[MAX_LENGTH], aux2[MAX_LENGTH];
	char path1[MAX_LENGTH], path2[MAX_LENGTH];
	
	
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

						printf("Linhas Iguais. \nabsolutePath:%s\n%s\n", path1, path2);
						

						// if files are duplicates
						if (compareContent(path1, path2) == 1){
							printf("Duplicado - igual conteudo\n");	
							//gerar hardLink e escrever em "hlinks.txt"
						}		   

						//if not
						else if (compareContent(path1, path2) == 0){
							printf("Nao duplicado - diferente conteudo\n");
						}
					}					
					else{
						//printf("%d %d", f1, f2);
						printf("Nao duplicado - linhas diferentes\n");
						printf("%s\n%s\n", str1, str2);
						break;
					}
					printf("%s%s\n", sub1, sub2);
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





int main(){

	//TEST IF A FILE IS A DUPLICATE
	isDuplicated("test.txt");
/*
	char sub1[MAX_LENGTH];
	getAbsolutePath("ana 9220 100775 /home/utyrea/Desktop/Trabalho1/lol.c Sat", sub1);
	printf("%s\n", sub1);

	char sub2[MAX_LENGTH];
	getAbsolutePath("duarte 9220 100775 /home/utyrea/Desktop/Trabalho1/cat.c Sat", sub2);
	printf("%s\n", sub2);

	char sub3[MAX_LENGTH];
	getAbsolutePath("duarte 9220 100775 /home/utyrea/Desktop/Trabalho1/lsdir.c Sat", sub3);
	printf("%s\n", sub3);

	//compareContent(sub2, sub1);*/


}
