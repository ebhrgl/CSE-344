#include <errno.h>
#include <pthread.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>
#include <dirent.h>
#include <unistd.h> 
#include <sys/stat.h>
#include <sys/wait.h>

#define BLKSIZE PIPE_BUF
#define MILLION 1000000L
#define D_MILLION 1000000.0
#define MAXNAME 80
#define R_FLAGS O_RDONLY
#define W_FLAGS (O_WRONLY | O_CREAT)
#define W_PERMS (S_IRUSR | S_IWUSR)
#define BUFSIZE 100

typedef struct {
int infd;
int outfd;
char filename[PATH_MAX];
} buffer_t;

static buffer_t buffer[BUFSIZE];
static pthread_mutex_t bufferlock = PTHREAD_MUTEX_INITIALIZER;
static int bufin = 0;
static int bufout = 0;
static int doneflag = 0;
static pthread_cond_t items = PTHREAD_COND_INITIALIZER;
static pthread_cond_t slots = PTHREAD_COND_INITIALIZER;
static int totalitems = 0;
static char destArray[BUFSIZE];

typedef struct {
	int args[3];
	pthread_t tid;
} copy_t;

static int *bytesp;
static copy_t *copies;
static int error;
static int i;
static int numcopiers;
static int totalbytes = 0;
static int bufsize=0;
struct sigaction sigact;
int directory(char* directName);
int is_directory(char *Path);
void *consumer(void *arg);
int is_nonRegular(char *Path);
void sighandler(int signo);
int main(int argc, char *argv[]) 
{
	long timedif;
	struct timeval tpend;
	struct timeval tpstart;
	
	if (argc != 5) {
		fprintf(stderr, "Usage: %s infile outfile copies\n", argv[0]);
		return 1;
	}

	numcopiers = atoi(argv[1]);
	bufsize = atoi(argv[2]);
	strcpy(destArray, argv[4]);

	if (gettimeofday(&tpstart, NULL)) 
	{
		fprintf(stderr, "Failed to get start time\n");
		return 1;
	}
	sigemptyset(&(sigact.sa_mask));
	sigact.sa_handler = sighandler;
	sigaction(SIGINT,&sigact,NULL);

	directory(argv[3]);

	if (gettimeofday(&tpend, NULL)) 
	{
		fprintf(stderr, "Failed to get end time\n");
		return 1;
	}
	timedif = MILLION*(tpend.tv_sec - tpstart.tv_sec) + tpend.tv_usec - tpstart.tv_usec;
	printf("The function_to_time took %ld microseconds\n", timedif);

	return 0;
}
void sighandler(int signo){
	printf("SIGINT(C) \n");
	doneflag=1;
}
/*Bu bölüm için kitabin MULTICOPY programindan yararlandim*/
int directory(char* directName)
{
	int flag=0;
	DIR *pDir=NULL;
	struct dirent *pDirent;
	char *currPath;
	char path[PATH_MAX];
	char pathd[PATH_MAX];
	buffer_t item;

	/*Dosyanin acilma durumunu kontrol ediyorum*/
	if((pDir = opendir(directName)) == NULL)
	{
		perror("Directory couldn't be opened.");
		return -1;	/* there is no words if directory cannot open */
	}

	while ((pDirent = readdir(pDir)) != NULL)
	{
		if (strcmp(pDirent->d_name, ".") != 0 && strcmp(pDirent->d_name, "..") != 0 &&  pDirent->d_name[strlen(pDirent->d_name) - 1] != '~')
        {
			/*path imi olusturuyorum*/				 
			strcpy(path,directName);
			strcat(path,"/");
			strcat(path,pDirent->d_name);
			//printf("path: %s\n",path);

			if(is_directory(path)==1 || is_nonRegular(path)==1)
			{	
				directory(path);
			}	
				
			else {
				strcpy(pathd,destArray);
				strcat(pathd,"/");
				strcat(pathd,pDirent->d_name);
				//printf("path: %s\n",pathd);

				if ((copies = (copy_t *)calloc(numcopiers, sizeof(copy_t))) == NULL) {
					perror("Failed to allocate copier space");
					return 1;
				}
				
				/* open the source and destination files and create the threads */
				for (i = 0; i < numcopiers; i++) 
				{
					copies[i].tid = pthread_self();
					putitem(item.filename);
					/* cannot be value for new thread */
					if (snprintf(item.filename, MAXNAME, "%s", path) == MAXNAME) 
					{
						fprintf(stderr, "Input filename %s", path);
						continue;
					}

					if ((copies[i].args[0] = open(item.filename, R_FLAGS)) == -1)
					{
				
						fprintf(stderr, "Failed to open source file %s: %s\n",item.filename, strerror(errno));
						continue;
					}
				

					if (snprintf(item.filename, MAXNAME, "%s", pathd) == MAXNAME) 
					{
						fprintf(stderr, "Output filename %s", destArray);
						continue;
					}

					if ((copies[i].args[1] = open(item.filename, W_FLAGS, W_PERMS)) == -1) 
					{
						fprintf(stderr, "Failed to open destination file %s: %s\n",
						item.filename, strerror(errno));
						continue;
					}

					if (error = pthread_create((&copies[i].tid), NULL,consumer, copies[i].args)) 
					{
						fprintf(stderr, "Failed to create thread: %s\n",strerror(error));
						copies[i].tid = pthread_self();
					/* cannot be value for new thread */
					}
				}
			
				/* wait for the threads to finish and report total bytes */
				for (i = 0; i < numcopiers; i++)
				{
					if (pthread_equal(copies[i].tid, pthread_self()))
					/* not created */
					continue;
					if (error = pthread_join(copies[i].tid, (void**)&bytesp)) 
					{
						fprintf(stderr, "Failed to join thread %d\n", i);
						continue;
					}

					if (bytesp == NULL) {
						fprintf(stderr, "Thread %d failed to return status\n", i);
						continue;
					}
					printf("Thread %d copied %d bytes from %s to %s\n",i, *bytesp, directName, destArray);
					totalbytes += *bytesp;
				}
				printf("Total bytes copied = %d\n", totalbytes);
				
			}
		}
	}

	pthread_mutex_lock(&bufferlock);
	flag = doneflag;
	pthread_mutex_unlock(&bufferlock);
	while((closedir(pDir) == -1) && (errno == EINTR));
	return 1;
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

/*Directory kontrolu yapilir.*/
int is_directory(char *Path)
{
	struct stat statbuf;
	stat(Path, &statbuf);

	if(S_ISDIR(statbuf.st_mode))
	{
		return 1;
	}
	return 0;		
}
/*Bu kod için kitaptan yararlandim */
int getitem(buffer_t *itemp) 
{/* remove an item from buffer and put in itemp */
	int error;
	if (error = pthread_mutex_lock(&bufferlock))
	return error;
	while ((totalitems <= 0) && !error && !doneflag)
	error = pthread_cond_wait (&items, &bufferlock);
	if (error) 
	{
		pthread_mutex_unlock(&bufferlock);
		return error;
	}
	if (doneflag && (totalitems <= 0)) 
	{
		pthread_mutex_unlock(&bufferlock);
		return ECANCELED;
	}
	*itemp = buffer[bufout];
	bufout = (bufout + 1) % bufsize;
	totalitems--;
	if (error = pthread_cond_signal(&slots)) 
	{
		pthread_mutex_unlock(&bufferlock);
		return error;
	}
	return pthread_mutex_unlock(&bufferlock);
}
/*Bu kod için kitaptan yararlandim */
int putitem(buffer_t item) 
{
/* insert an item in the buffer */
	int error;
	if (error = pthread_mutex_lock(&bufferlock))
	return error;
	while ((totalitems >= bufsize) && !error && !doneflag)
	error = pthread_cond_wait (&slots, &bufferlock);
	if (error) 
	{
		pthread_mutex_unlock(&bufferlock);
		return error;
	}
	if (doneflag) {
		/* consumers may be gone, don't put item in */
		pthread_mutex_unlock(&bufferlock);
		return ECANCELED;
	}
	buffer[bufin] = item;
	bufin = (bufin + 1) % BUFSIZE;
	totalitems++;
	if (error = pthread_cond_signal(&items)) 
	{
		pthread_mutex_unlock(&bufferlock);
		return error;
	}
	return pthread_mutex_unlock(&bufferlock);
}
/*Bu kod için kitaptan yararlandim */
void *consumer(void *arg)
{
	int bytesread;
    int totalbytes = 0;
	int *fd;
	fd = (int *)arg;
	buffer_t itemp;
	getitem(&itemp);

    while ((bytesread = readwrite(fd[0],  fd[1])) > 0)
      totalbytes += bytesread;
 
	fd[2] = totalbytes;
	close(fd[0]);
	close(fd[1]);
	return fd + 2;
}

/*Kitabın restart kütüphanesinden fonksiyonlar kullandım*/
/* Restart versions of traditional functions */

int r_close(int fildes) {
   int retval;
   while (retval = close(fildes), retval == -1 && errno == EINTR) ;
   return retval;
}

ssize_t r_read(int fd, void *buf, size_t size) {
   ssize_t retval;
   while (retval = read(fd, buf, size), retval == -1 && errno == EINTR) ;
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

/* Utility functions */

int readwrite(int fromfd, int tofd) {
   char buf[BLKSIZE];
   int bytesread;

   if ((bytesread = r_read(fromfd, buf, BLKSIZE)) < 0)
      return -1;
   if (bytesread == 0)
      return 0;
   if (r_write(tofd, buf, bytesread) < 0)
      return -1;
   return bytesread;
}

