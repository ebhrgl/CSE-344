// Write CPP code here 
#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <signal.h>
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


#define MAX 80 
#define SA struct sockaddr  

struct client_t
{
	char pathNameDest[1024];
	char pathNameSource[1024];
	int size;
};

struct client_t clients;
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
int portNum=0;
char* ip_address;
pid_t client_pid;
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

//Kitap kodundan yararlandım
void *copyfilepass(void *arg)
{
	int *argint;
	argint = (int *)arg;
	argint[2] = copyfile(argint[0], argint[1]);
	r_close(argint[0]);
	r_close(argint[1]);
	return argint + 2;
}


int copyfile(int fromfd, int tofd) {
   int bytesread;
   int totalbytes = 0;

   while ((bytesread = readwrite(fromfd, tofd)) > 0)
      totalbytes += bytesread;
   return totalbytes;
}




//kitabın kodundan yararlandım
int directory(char* directName)
{
	int flag=0;
	DIR *pDir=NULL;
	struct dirent *pDirent;
	char *currPath;
	char path[PATH_MAX];
	char pathd[PATH_MAX];
	buffer_t item;
	char temp[1024];
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

					if (error = pthread_create((&copies[i].tid), NULL,copyfilepass, copies[i].args)) 
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

void server_sinyal_handler(int signumber)
{
	if(signumber == SIGINT)
	{
		doneflag=1;
		fprintf(stdout, "Client has got a SIGINT signal!\n");
		kill(client_pid,SIGINT);
	}
					
	else if(signumber == SIGTERM)
	{
		fprintf(stdout, "Client[%ld] has got a SIGTERM signal!\n", (long)getpid());
		kill(client_pid,SIGTERM);	
	}
		
	else if(signumber == SIGQUIT)
	{
		
	
		fprintf(stdout, "Client has got a SIGQUIT signal!\n");	
		kill(client_pid,SIGQUIT);	
	}
		
	else if(signumber == SIGUSR1)
	{	
		exit(EXIT_FAILURE);
			
	}

	else if(signumber == SIGFPE)
	{
		if(client_pid)
			kill(client_pid, SIGFPE);
			exit(EXIT_FAILURE);		
	}	


		kill(0, signumber);
	
	if(client_pid)
		kill(client_pid, SIGUSR1);
	
	exit(signumber);
}
  //Socket kodu için https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/ sitesinden yararlandım.
int main(int argc, char *argv[]) 
{ 
    int sockfd, connfd; 
	int x,n,y,k;
    struct sockaddr_in servaddr, cli; 
    struct dirent *pDir;
	struct sigaction act;
	if (argc != 4) {
		fprintf(stderr, "Usage: %s\n", argv[0]);
        fprintf(stderr, "\n<Directoryname>");
        fprintf(stderr, "\n<ipAddress>");
        fprintf(stderr, "\n<Portnumber>");
        exit(EXIT_FAILURE);
	}

	act.sa_handler = server_sinyal_handler; 
	act.sa_flags = 0;

	if ( (sigemptyset(&act.sa_mask) == -1) || (sigaction(SIGINT, &act, NULL) == -1) 
		|| (sigaction(SIGTERM, &act, NULL) == -1) || (sigaction(SIGQUIT, &act, NULL) == -1)
		|| (sigaction(SIGUSR1, &act, NULL) == -1)|| (sigaction(SIGFPE, &act, NULL) ==-1))		
	{
		perror("Server: Failed to set signal handler");
		return 1; 
	}

	signal(SIGINT, server_sinyal_handler);	 
	signal(SIGTERM, server_sinyal_handler);
	signal(SIGQUIT, server_sinyal_handler);
	signal(SIGUSR1, server_sinyal_handler);
	signal(SIGFPE, server_sinyal_handler);
	
	client_pid = getpid();
	strcpy(clients.pathNameSource,argv[1]);
	
	portNum = atoi(argv[3]);
	ip_address = argv[2];

    // socket create and varification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created..\n"); 
    	bzero(&servaddr, sizeof(servaddr)); 
  
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    //servaddr.sin_addr.s_addr = inet_addr(AF_INET,argv[2],&servaddr.sin_addr); 
	inet_pton(AF_INET, ip_address, &servaddr.sin_addr.s_addr);

    servaddr.sin_port = htons(portNum); 
  
    // connect the client socket to server socket 
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
        printf("connection with the server failed...\n"); 
        exit(0); 
    } 
    else
        printf("connected to the server..\n"); 
	while(1)
	{
		r_read(sockfd,clients.pathNameDest, sizeof(char)*1024);
		r_read(sockfd,&clients.size, sizeof(clients.size));
		r_write(sockfd,&clients.pathNameSource,sizeof(char)*1024);
		r_write(sockfd,&client_pid,sizeof(pid_t));
		//printf(" eeeeee   %d\n",clients.size);
		numcopiers = clients.size;
		//printf(" eeeeee   %s\n",clients.pathNameDest);
		strcpy(destArray, clients.pathNameDest);
	
		directory(argv[1]);
	}
    close(sockfd); 
} 

/*Bu kod için kitaptan yararlandim */
int putitem(buffer_t item) 
{
/* insert an item in the buffer */
	int error;
	if (error = pthread_mutex_lock(&bufferlock))
	return error;
	while ((totalitems >= BUFSIZE) && !error && !doneflag)
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
