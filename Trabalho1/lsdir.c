#include <stdio.h>
/*For directory related functions (DIRectory ENTries)*/
#include <dirent.h>	

/*stat function includes*/
/*See: http://linux.die.net/man/2/stat*/
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

/*for time functions and structs*/
#include <time.h>

/*include open functions*/
#include <fcntl.h>


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

/*TODO: TO GET FILE INFORMATION USE STAT() FUNCTION*/

/* Saves a file's info to a RFile struct. DON'T FORGET TO FREE MEMORY AFTER USING IT! @see RFile.
 * */
void getInfoFromFile (char* filePath, RFile* file)
{
	struct stat s;		// get file data
	
	stat (filePath, &s);
	
	file->size = (long long) s.st_size;
	file->mode = (unsigned long) (s.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO));
	file->lastMDate = s.st_mtime;	
	file->absolutePath = filePath;
}

/* Converts RFile data to a string that can be saved in a .txt file.
 * */
void rFileToString(RFile file, char* fileInfo)
{
	sprintf(fileInfo, "%s %lld %lo %s %s", file.name, file.size, file.mode, file.absolutePath, ctime(&file.lastMDate));	// %lo prints in long octal
}

/*
 * Saves to a text file the name of the regular files present in the directory.
 * */
void writeFilesFromDir(char* directory){
	DIR *dir;
	struct dirent *ent;
	
	pid_t childPids[30];
	int pidCounter = 0;
	char currentPath[PATH_MAX];
	 
	
	if ((dir = opendir (directory)) != NULL) // if teh directory can be opened
	{
		FILE *files;

		files = fopen("/tmp/files.txt", "a");	//append
		
		/*Write all file names to a file.*/
		while ((ent = readdir (dir)) != NULL) // while there are files to read
		{
			struct stat s;
			
			sprintf(currentPath, "%s/%s", directory, ent->d_name);
			
			if(stat(currentPath, &s))	// if it's not 0 (if it's not sucessful)
			{
				perror("stat failed");
				exit (-1);
			} 
			
			// if it's a regular file
			if (S_ISREG(s.st_mode))
			{		 
					RFile* file = (RFile*)malloc(sizeof(RFile));
					char fileInfo[1024] = "";
					
					getInfoFromFile(currentPath, file);
					file->name = ent->d_name;
					
					rFileToString(*file, fileInfo);
					
					fputs(fileInfo, files);
					
					free(file);
					
					continue;
			}
			
			// if it's a directory
			if(S_ISDIR(s.st_mode) && (strcmp(".", ent->d_name) != 0) && (strcmp("..", ent->d_name) != 0))
			{	
				childPids[pidCounter] = fork();	// add child ID to array
				
				if(childPids[pidCounter] < 0)
				{
					perror("Fork failed.");
					exit(-1);
				}
				
				if(childPids[pidCounter]== 0) // child
				{	
					execl("./lsdir", "lsdir", currentPath, (char *)0);
					perror("execl failed");
					
					exit(0);
					
				}
				
				if(childPids[pidCounter]> 0) // father
				{	
					//continue
				}
				
				pidCounter++;
			}
			
		}
		
		fclose(files);
		closedir(dir);
		
	} else {
		/* could not open directory */
		perror ("Could not open directory.");
		exit(-2);
	}
	
	/*Wait for child processes*/
	int counter = 0;
	
	for (; counter < pidCounter; counter++)
	{
		waitpid(childPids[counter], NULL, 0);
	}
}


int main(int argc, char* argv[])
{	
	writeFilesFromDir(argv[1]);
	
	return 0;	
}
