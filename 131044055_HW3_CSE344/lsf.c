#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {

	DIR *pDir = NULL;
    struct dirent *pDirent;
	struct stat st;

	/*Dosyanin acilma durumunu kontrol ediyorum*/
	if((pDir = opendir(".")) == NULL)
	{
		perror("Directory couldn't be opened.");
		return -1;	/* there is no words if directory cannot open */
	}
	
	else {
		printf(" Type\tPermissions\tSize\t   File Name\n");
		while ((pDirent = readdir(pDir)) != NULL) {
			if (strcmp(pDirent->d_name, ".") != 0 && strcmp(pDirent->d_name, "..") != 0 &&  pDirent->d_name[strlen(pDirent->d_name) - 1] != '~') {
				stat(pDirent->d_name, &st);
				
				printf((S_ISDIR(st.st_mode)) ? "  d" : S_ISLNK(st.st_mode) ? "  l" : "  -");

				printf((st.st_mode & S_IRUSR) ? "\t r" : "\t -");
				printf((st.st_mode & S_IWUSR) ? "w" : "-");
				printf((st.st_mode & S_IXUSR) ? "x" : "-");
				printf((st.st_mode & S_IRGRP) ? "r" : "-");
				printf((st.st_mode & S_IWGRP) ? "w" : "-");
				printf((st.st_mode & S_IXGRP) ? "x" : "-");
				printf((st.st_mode & S_IROTH) ? "r" : "-");
				printf((st.st_mode & S_IWOTH) ? "w" : "-");
				printf((st.st_mode & S_IXOTH) ? "x" : "-");
				printf("\t%ld", st.st_size);
				printf("\t   %s\n", pDirent->d_name);

				/*if(S_ISREG(st.st_mode)){
					sprintf(fileType,"Regular File\0");
				}else if(S_ISDIR(st.st_mode)){
					sprintf(fileType,"Directory\0");
				}else if(S_ISCHR(st.st_mode)){
					sprintf(fileType,"Character Device\0");
				}else if(S_ISBLK(st.st_mode)){
					sprintf(fileType,"Block Device\0");
				}else if(S_ISFIFO(st.st_mode)){
					sprintf(fileType,"FIFO\0");
				}else if(S_ISLNK(st.st_mode)){
					sprintf(fileType,"Symbolic Link\0");
				}else if(S_ISSOCK(st.st_mode)){
					sprintf(fileType,"Socket\0");
				}else{
					sprintf(fileType,"Unknown File Type\0");
				}*/
				
				printf("     //%lld bytes//",(long long)st.st_size);
				printf("     %s\n",pDirent->d_name);

			}
		}
	}
	
	closedir(pDir);
	exit(EXIT_SUCCESS);
}

 	
	
