#include <stdio.h>
#include <dirent.h>	/*For directory related functions (DIRectory ENTries)*/

/*stat function includes*/
/*See: http://linux.die.net/man/2/stat*/
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <unistd.h>

/*TODO: TO GET FILE INFORMATION USE STAT() FUNCTION*/

/*Writes file info to a files.txt */ 
int writeFileInfo(char* directory, int level){

	DIR *dir;
	struct dirent *ent;
	struct stat s;
	pid_t  pid;
	
	
	dir = opendir (directory);
	
	chdir(directory);  /*Selects the directory that will be passed as an argument */

	if (dir != NULL){
			
			//FONTE DE INF: http://linux.die.net/man/2/fstat
			
			FILE *files;

			files = fopen("/tmp/files.txt", "w++"); // opens or creates or updates for writing only
																							//instead of using /tmp/ use mkstemp()

			while ((ent = readdir (dir)) != NULL) {

				stat(ent->d_name, &s); 
				
				if (S_ISREG(s.st_mode))  /*  	ou    if(ent->d_type == DT_REG) */

						/*nome - tamanho - permissoes - data de modificação */
						fprintf(files, "%s %lld %lo %s \n", ent->d_name, (long long)s.st_size,(unsigned long)s.st_mode, ctime(&s.st_mtime));

				if (S_ISDIR(s.st_mode)){   	/*		ou		if (ent->d_type == DT_DIR) */
					
					int status;
					
					pid = fork();
						
						if (pid == 0){	// child

							//chamar função (recursivamente)	ou ver subdiretorios na getDirectoryFilesInfo()
							char path[1024];
							int len = snprintf(path, sizeof(path)-1, "%s/%s", directory, ent->d_name);
							path[len] = 0;
							closedir(dir);
							writeFileInfo(path, level+1);
						}
						else
						{
							wait(&status);
							continue;
						}
						
				}
				
				
			}
		
	//checks the number of subdirectories ; not needed
		//fprintf(files, "%d \n", num_dir); 
		
		fclose(files);			
		closedir(dir);
	}

	else {
		/* could not open directory */
		perror ("");
		return -1;
	}

	return 0;

}

int main(){
	
	char* filePath = "/home/utyrea/Desktop/SOPE/Trabalho1/test";
	char* testDirectory = "/home/jazz/Desktop/SOPE-Feup";
	
	writeFileInfo(testDirectory, 0);
}
