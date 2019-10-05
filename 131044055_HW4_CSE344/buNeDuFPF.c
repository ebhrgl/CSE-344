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
#define BUFSIZE 10 
#define FIFO_PERM (S_IRUSR | S_IWUSR)
#define FIFO_MODES O_RDONLY
#define READ_FLAGS O_RDONLY

typedef struct{
  pid_t mypid;
  int fd[2];
  char fn[PATH_MAX]; 
}process;

process *forkArr=NULL;
/*Static value*/
static int sizeFile = 0;
static int temp=0;
static long pid = 0;
static long ppid = 0;

/*Program Fonksiyonlari*/
int postOrderApply (char *path, int pathfun (char *path1));
int sizepathfun (char *path);
pid_t r_wait(int *stat_loc);
/*Dosya kontrollerini iceren Fonksiyonlar*/
int is_directory(char *Path);
int is_regular(char *Path);
int is_nonRegular(char *Path);
ssize_t r_write(int fd, void *buf, size_t size);
ssize_t r_read(int fd, void *buf, size_t size);


int main(int argc, char *argv[])
{
	
	int size=0;
	
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
		printf("PID  %ld\t ", pid);
		printf("Total File Size: %d\n", size);
		printf("Main process is:  %ld\n ", ppid);
		
    }

	/*Tüm directory lerin sizeof toplamı için girisi saglayan komut satiri*/
	if(argc== 3)
	{
		if(strcmp(argv[1],"-z")==0)
		{
			temp=1;
			size = postOrderApply(argv[2],sizepathfun);
			printf("PID  %ld\t ", pid);
			printf("Total File Size: %d\n", size);
			printf("Main process is:  %ld\n ", ppid);
			
		}	
	}  

    return 0;
}

/*Tüm dosya kontrol işlemleri ve genel işlemler bu fonksiyon içind yapilir*/
int postOrderApply (char *path, int pathfun (char *path1))
{
    DIR *pDir = NULL;
    struct dirent *pDirent;
    int dirSizeOf = 0;
    char filePath[PATH_MAX];
	int dir_size = 0;
	pid_t pidchild;
	char bufin[BUFSIZE] = "empty";
	char bufout[] = "hello";
	int bytesin;
	int fd[2]; 
	char buf[BUFSIZE];
	int fdFifo;
	int rval;
	ssize_t strsize;
	int chilsize = 0,i;
	int countFile=0;
    char fifoName[PATH_MAX] = "131044055sizes";
	pid = (long)getpid();
	ppid =(long)getppid();

	
	/*Dosyanin acilma durumunu kontrol ediyorum*/
	if((pDir = opendir(path)) == NULL)
	{
		perror("Directory couldn't be opened.");
		return -1;	/* there is no words if directory cannot open */
	}

	
	while((pDirent = readdir(pDir)) != NULL)
    {
		if (strcmp(pDirent->d_name, ".") != 0 && strcmp(pDirent->d_name, "..") != 0 &&  pDirent->d_name[strlen(pDirent->d_name) - 1] != '~')
        {
			strcpy(filePath,path);
			strcat(filePath,"/");
			strcat(filePath,pDirent->d_name);
            if(is_directory(filePath)==1) 
            {
				++chilsize;
			} 
			else if(is_regular(filePath)==1)
		    {			
				++chilsize;
		    }

			else if(is_nonRegular(filePath)==1)
			{	
				++chilsize;
			}
		}
	}
	/*Çocuk sayısı boyutunda dizi oluşturulup ilklendirme yapıldı.*/
    forkArr=(process*)malloc(sizeof(process)*chilsize);
	for(i=0;i<chilsize;++i)
	{
		forkArr[i].fd[0]=-1;
		forkArr[i].fd[1]=-1;
	}
	
	/*beliritilen klasöre konumlanır*/
	rewinddir(pDir);
	countFile=-1;
    while ((pDirent = readdir(pDir)) != NULL)
    {
        if (strcmp(pDirent->d_name, ".") != 0 && strcmp(pDirent->d_name, "..") != 0 &&  pDirent->d_name[strlen(pDirent->d_name) - 1] != '~')
        { 
			strcpy(filePath,path);
			strcat(filePath,"/");
			strcat(filePath,pDirent->d_name);
			
		    if (is_directory(filePath)==1)
		    { 
					++countFile;
					forkArr[countFile].mypid = getpid();
					sprintf(fifoName,"%ld-%d.131044055sizes",(long)forkArr[countFile].mypid,countFile);
					sprintf(forkArr[countFile].fn,"%s",fifoName); 

					if(mkfifo(fifoName,FIFO_PERM)==-1){
						if(errno != EEXIST){
							fprintf(stderr, "[%ld]:failed to create named pipe %s: %s\n",
		                            (long)forkArr[countFile].mypid, fifoName, strerror(errno));
							if(chilsize != 0)
							{
								free(forkArr);
							}
							closedir(pDir);
							exit(-1);
						}
					}
				
					if (pipe(fd) == -1) {
						perror("Failed to create the pipe");
						return 1;
					}
					bytesin = strlen(bufin); 
	
                    pidchild = fork();

					if(pidchild == -1 && (errno != EINTR))
                    {
                         perror("Failed to fork\n");
                         closedir(pDir);
                         return 1;
                    }

                    if (pidchild == 0) {
                      	
		                while (((forkArr[countFile].fd[1] = open(fifoName, O_WRONLY)) == -1) && (errno == EINTR));
						if (forkArr[countFile].fd[1] == -1) {
							fprintf(stderr, "[%ld]:failed to open named pipe %s for write: %s\n",
									(long) getpid(), fifoName, strerror(errno));
							exit(-1);
						}
					
						if(chilsize != 0)
						{
							free(forkArr);
						}
						
						
						// send the value on the write-descriptor.
					    dir_size = postOrderApply(filePath,pathfun);
						rval = r_write(fdFifo, buf, strsize);
						if (rval != strsize) {
							fprintf(stderr, "[%ld]:failed to write to pipe: %s\n",
							(long)getpid(), strerror(errno));
							return 1;
						}

						if(temp==0)
						{
							printf("PID: %ld\t", pid);
							printf("SIZE: %d\t", dir_size);
						}

						if(temp==1){

  						   close(fd[0]);
						   sprintf(bufout, "%d\t", dir_size);
						   dirSizeOf += dir_size;
						   write(fd[1], bufout, strlen(bufout)+1);
						   close(fd[1]);

						}
						
						// close the write descriptor
						
						closedir(pDir);
						
						printf("PATH: %s\n", filePath);
						close(forkArr[countFile].fd[1]);
                        exit(0);
                    }
					else{

						while (((forkArr[countFile].fd[0] = open(fifoName, READ_FLAGS)) == -1) && (errno == EINTR));
						if (forkArr[countFile].fd[0] == -1) {
							fprintf(stderr, "[%ld]:failed to open named pipe %s for read: %s\n",
									(long) getpid(), fifoName, strerror(errno));
							free(forkArr);
							closedir(pDir);
							return -1;
						}
						forkArr[countFile].mypid = pidchild; 

						/*rval = r_read(fdFifo, buf, BUFSIZE);
						if (rval == -1) {
							fprintf(stderr, "[%ld]:failed to read from pipe: %s\n",
							(long)getpid(), strerror(errno));
							return 1;
						}*/
						close(forkArr[countFile].fd[0]);
						//unlink(forkArr[i].fn);
						if(temp==1){
							close(fd[1]);
							bytesin = read(fd[0], bufin, BUFSIZE);
							fprintf(stderr, "PID %ld\t SIZE %s",pid, bufin); 
							// close the read-descriptor
						
						}
					}
                    		
		    }

		    else if(is_regular(filePath)==1)
		    {			
				dirSizeOf += pathfun(filePath);

		    }

			else if(is_nonRegular(filePath)==1)
			{	
				printf("PID  %ld\t ", pid);
				dir_size = -1;
				printf("SIZE  %d\t ", dir_size);
				printf("PATH: %s\n", filePath);
				printf("This is a Special File\n");		
				 for(i=0;i<chilsize;++i)
				 {
					unlink(forkArr[i].fn);
				 }
		
			}
		}
    }
 	 close(fd[0]);
	 while (r_wait(NULL) > 0); 
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
pid_t r_wait(int *stat_loc) {
    int retval;
    while (((retval = wait(stat_loc)) == -1) && (errno == EINTR));
    return retval;
}
ssize_t r_write(int fd, void *buf, size_t size) {
   char *bufp;
   size_t bytestowrite;
   ssize_t byteswritten;
   size_t totalbytes;

   for (bufp = buf, bytestowrite = size, totalbytes = 0;
        bytestowrite > 0;
        bufp += byteswritten, bytestowrite -= byteswritten) {
      byteswritten = write(fd, bufp, bytestowrite);
      if ((byteswritten == -1) && (errno != EINTR))
         return -1;
      if (byteswritten == -1)
         byteswritten = 0;
      totalbytes += byteswritten;
   }
   return totalbytes;
}

ssize_t r_read(int fd, void *buf, size_t size) {
   ssize_t retval;
   while (retval = read(fd, buf, size), retval == -1 && errno == EINTR) ;
   return retval;
}
