#include <stdio.h>
#include <dirent.h>	/*For directory related functions (DIRectory ENTries)*/

/*stat function includes*/
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/*TODO: TO GET FILE INFORMATION USE STAT() FUNCTION*/

char* getInfoFromFile (char* filePath)
{
	struct stat *s;
	
	stat (filePath, s);
	
	/*Use "/" do separate data.*/
	
	return NULL;
}

int writeFilesFromDir(char* directory){
	DIR *dir;
	struct dirent *ent;
	
	if ((dir = opendir (directory)) != NULL) 
	{
		FILE *files;

		files = fopen("files.txt", "w");	// opens or creates for writing only
	
		/*Write all file names to a file.*/
		while ((ent = readdir (dir)) != NULL) 
		{
			char* fileName = ent->d_name;

			fputs(fileName, files);
			fputs("\n", files);
		}
		
		fclose(files);
			
		closedir(dir);
	} else {
		/* could not open directory */
		perror ("");
		return -1;
	}

	return 0;
}

int main()
{
	char* testDirectory = "/usr/users2/mieic2014/up201403526/Downloads";
	
	writeFilesFromDir(testDirectory);
	
}
