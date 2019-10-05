/*******************************/
/*      131044055              */
/*    Eda BAHRIOGLU            */
/*******************************/
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

/*Static value*/
static int sizeFile = 0;
static int temp=0;
static long pid = 0;
static long ppid = 0;
static int childsize = 0;
static char* file = "131044055sizes.txt";
/*Program Fonksiyonlari*/
int postOrderApply (char *path, int pathfun (char *path1));
int sizepathfun (char *path);

/*Dosya kontrollerini iceren Fonksiyonlar*/
int is_directory(char *Path);
int is_regular(char *Path);
int is_nonRegular(char *Path);

int main(int argc, char *argv[])
{
	FILE *fp;
	int size=0;
	fp = fopen(file, "a");
	

	/*usage*/
    if (argc != 3 && argc !=2)
    {
        fprintf(stderr, "USAGE: %s HATA\n", argv[0]);
        return 1;
    }

	/*Sadece dosya sizeof toplamı için girisi saglayan komut satiri*/
	if(argc == 2)
    {
        size = postOrderApply(argv[1],sizepathfun);
		fprintf(fp,"PID  %ld\t ", pid);
		fprintf(fp,"Total File Size: %d\n", size);
		fprintf(fp,"Main process is:  %ld\n ", ppid);
		fprintf(fp," %d child processes created.\n ", childsize);
		printf("PID  %ld\t ", pid);
		printf("Total File Size: %d\n", size);
		printf("Main process is:  %ld\n ", ppid);
		printf(" %d child processes created.\n ", childsize);
    }

	/*Tüm directory lerin sizeof toplamı için girisi saglayan komut satiri*/
	if(argc== 3)
	{
		if(strcmp(argv[1],"-z")==0)
		{
			temp=1;
			size = postOrderApply(argv[2],sizepathfun);
			fprintf(fp,"PID  %ld\t ", pid);
			fprintf(fp,"Total Directory Size: %d\n", size);
			fprintf(fp,"Main process is: %ld\n ", ppid);
			fprintf(fp," %d child processes created.\n ", childsize);
			printf("PID  %ld\t ", pid);
			printf("Total File Size: %d\n", size);
			printf("Main process is:  %ld\n ", ppid);
			printf(" %d child processes created.\n ", childsize);
		}	
	}  

	fclose(fp);
    return 0;
}

/*Tüm dosya kontrol işlemleri ve genel işlemler bu fonksiyon içind yapilir*/
int postOrderApply (char *path, int pathfun (char *path1))
{
    DIR *pDir = NULL;
    struct dirent *pDirent;
	struct flock lock;
	FILE *fp;
	int fd;
	char arr[100000];
    int dirSizeOf = 0;
    char filePath[PATH_MAX];
	int dir_size = 0;
	pid_t pidchild;
	pid_t parent_child;
	pid = (long)getpid();
	ppid =(long)getppid();
	
	/*Dosyanin acilma durumunu kontrol ediyorum*/
	if((pDir = opendir(path)) == NULL)
	{
		perror("Directory couldn't be opened.");
		return -1;	/* there is no words if directory cannot open */
	}

    while ((pDirent = readdir(pDir)) != NULL)
    {
        if (strcmp(pDirent->d_name, ".") != 0 && strcmp(pDirent->d_name, "..") != 0 &&  pDirent->d_name[strlen(pDirent->d_name) - 1] != '~')
        { 
			strcpy(filePath,path);
			strcat(filePath,"/");
			strcat(filePath,pDirent->d_name);

		    if (is_directory(filePath)==1)
		    {
				pidchild = fork();
				
				if (pidchild >= 0) 
				{
					childsize++;
					
					if (pidchild == 0) 
					{

						if ((fd = open(file, O_CREAT | O_WRONLY | O_APPEND | O_CREAT, 0600)) == -1)  {
							perror("open");
							exit(1);
						}
		
						fp = fopen(file, "a");
						dir_size = postOrderApply(filePath,pathfun);
						
						//file lock
						
						memset (&lock, 0, sizeof(lock));
						lock.l_type = F_WRLCK;
						if (fcntl(fd, F_SETLKW, &lock) == -1) {
							perror("fcntl");
							exit(1);
						}
						
						fprintf(fp,"PID  %ld\t ", pid);
						fprintf(fp,"SIZE: %d\t", dir_size);
						fprintf(fp, "PATH: %s\n", filePath);
						

						//file unlock
						lock.l_type = F_UNLCK;
						if (fcntl(fd, F_SETLK, &lock) == -1) {
							perror("fcntl");
							exit(1);
						}
						
						fclose(fp);
						close(fd);

						if(temp==1){

						   dirSizeOf += dir_size;
						}
					
						exit(0);
					}
					else
					{	/***** Parent *****/
						pidchild = waitpid(pidchild, NULL, 0);

						if ((fd = open(file, O_CREAT | O_WRONLY | O_APPEND | O_CREAT, 0600)) == -1)  {
							perror("open");
							exit(1);
						}
						fp = fopen(file, "r");
					
						//file lock
						
						memset (&lock, 0, sizeof(lock));
						lock.l_type = F_WRLCK;
						if (fcntl(fd, F_SETLKW, &lock) == -1) {
							perror("fcntl");
							exit(1);
						}
		
						//read from file
					
						while(fgets(arr,10000,fp)!=NULL)
						{
							printf("%s", arr);
						}
						
						//file unlock
						lock.l_type = F_UNLCK;
						if (fcntl(fd, F_SETLK, &lock) == -1) {
							perror("fcntl");
							exit(1);
						}
						
						fclose(fp);
						close(fd);
					}

				}
		    }

		    else if(is_regular(filePath)==1)
		    {		
				dirSizeOf += pathfun(filePath);		
		
		    }

			else if(is_nonRegular(filePath)==1)
			{
				fp = fopen(file, "a");
				fprintf(fp,"PID  %ld\t ", pid);
				fprintf(fp,"PATH: %s\n", filePath);
				fprintf(fp,"This is a Special File\n");
				
				fclose(fp);
			}
		}
    }
 	
	while((closedir(pDir) == -1) && (errno == EINTR));
    return dirSizeOf;
}

/*Dosya sizeof unu bulamyi sağlayan fonksiyondur.*/
int sizepathfun (char *path)
{
	 int size = 0;
	 struct stat st;
 	 lstat(path, &st);
     size = st.st_size;
	 return size;
}

/*Dosyanin directory olup olmadigini kontrol eder*/
int is_directory(char *Path)
{
	struct stat statbuf;
	lstat(Path, &statbuf);

	if(S_ISDIR(statbuf.st_mode))
	{
		return 1;
	}
	return 0;		
} 

/*Dosyanin regular olup olmadigini kontrol eder*/
int is_regular(char *Path)
{
	struct stat statbuf;
	lstat(Path, &statbuf);

	if(S_ISREG(statbuf.st_mode))
	{
		return 1;
	}
	return 0;		
} 

/*Dosyanin special dosya olup olmadiginin kontrolu yapilir*/
int is_nonRegular(char *Path)
{
	struct stat statbuf;
	lstat(Path, &statbuf);

	if(S_ISBLK(statbuf.st_mode) || S_ISCHR(statbuf.st_mode) || S_ISFIFO(statbuf.st_mode) || S_ISLNK(statbuf.st_mode) || S_ISSOCK(statbuf.st_mode))
	{
		return 1;
	}
	return 0;	
}


