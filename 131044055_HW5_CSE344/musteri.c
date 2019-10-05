#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <sys/time.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <dirent.h>
#include <time.h>
#include <sys/select.h>
#define FIFO_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define BLKSIZE PIPE_BUF
#define MILLION 1000000L
#define BUFSIZE 256
#define D_MILLION 1000000.0
#ifndef ETIME
#define ETIME ETIMEDOUT
#endif
#ifndef PIPE_BUF
#define PIPE_BUF 4096
#endif

pid_t pid;
static int mainFifofd2;
static int clientFifo;
static int clientFifoNum;
static int processNum = 0;
char serverFifoName[PATH_MAX] = "fifo";
char clientFifoName[PATH_MAX] = "clientfifo";
char clientFifoName2[PATH_MAX] = "clientfifo2";
void signal_handler_client(int signumber);
struct timeval add2currenttime(double seconds);
int copyfile(int fromfd, int tofd);
int r_close(int fildes);
int r_dup2(int fildes, int fildes2);
int r_open2(const char *path, int oflag);
int r_open3(const char *path, int oflag, mode_t mode);
ssize_t r_read(int fd, void *buf, size_t size);
pid_t r_wait(int *stat_loc);
pid_t r_waitpid(pid_t pid, int *stat_loc, int options);
ssize_t r_write(int fd, void *buf, size_t size);
ssize_t readblock(int fd, void *buf, size_t size);
int readline(int fd, char *buf, int nbytes);
ssize_t readtimed(int fd, void *buf, size_t nbyte, double seconds);
int readwrite(int fromfd, int tofd);
int readwriteblock(int fromfd, int tofd, char *buf, int size);
int waitfdtimed(int fd, struct timeval end);



int main(int argc, char *argv[]) {

	struct sigaction act;
	int buf[BUFSIZE];
	pid = getpid();
	int processSize=0;
	pid_t pidchild;
	char bufin[BUFSIZE];
	char bufout[] = "hello";
	int bytesin;
	int fd[2]; 
	
	int i=0;
	if (argc != 2) {
        fprintf(stderr, "Usage: %s <fifoname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

	signal(SIGINT,signal_handler_client);
	signal(SIGTERM,signal_handler_client); 
	signal(SIGUSR1,signal_handler_client);
	signal(SIGUSR2,signal_handler_client);
	signal(SIGQUIT,signal_handler_client);
	signal(SIGFPE, signal_handler_client); 
	signal(SIGALRM,signal_handler_client);

	processSize = atoi(argv[1]);
	if(mkfifo(clientFifoName2,FIFO_PERMS)==-1){ 
		if(errno != EEXIST){	
			fprintf(stderr, "[%ld]:failed to create named pipe %s: %s\n", (long)getpid(), clientFifoName2, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	if (pipe(fd) == -1) {
		perror("Failed to create the pipe");
		return 1;
	}
	bytesin = strlen(bufin); 

	for(i=0; i < processSize; i++)
	{

		
		pidchild = fork();

		if( pidchild == -1){
			fprintf(stderr,"timerServer failed to create fork. Error no : %s\n",strerror(errno));
			exit(EXIT_FAILURE);
		}
		if (pidchild == 0)
		{
			
			/*sinyal gönderdim*/
			act.sa_handler = signal_handler_client; 
			act.sa_flags = 0;

			if ((sigemptyset(&act.sa_mask) == -1) || (sigaction(SIGINT, &act, NULL) == -1)
				|| (sigaction(SIGTERM, &act, NULL) == -1) || (sigaction(SIGUSR1, &act, NULL) == -1)
				|| (sigaction(SIGUSR2, &act, NULL) == -1) || (sigaction(SIGQUIT, &act, NULL) == -1)
				|| (sigaction(SIGFPE, &act, NULL) == -1) || (sigaction(SIGALRM, &act, NULL) == -1))
			{
				perror("client: Failed to set signal handler");
				return 1;
			}
			signal(SIGINT,signal_handler_client);
			signal(SIGTERM,signal_handler_client); 
			signal(SIGUSR1,signal_handler_client);
			signal(SIGUSR2,signal_handler_client);
			signal(SIGQUIT,signal_handler_client);
			signal(SIGFPE, signal_handler_client); 
			signal(SIGALRM,signal_handler_client);

			//sprintf(clientFifoName,"%ld_fifo",(long)pid);
			/* create clientfifo */
			if(mkfifo(clientFifoName,FIFO_PERMS)==-1){ 
				if(errno != EEXIST){	
					fprintf(stderr, "[%ld]:failed to create named pipe %s: %s\n", (long)getpid(), clientFifoName, strerror(errno));
					exit(EXIT_FAILURE);
				}
			}

			
			mainFifofd2 = open(serverFifoName,O_RDONLY);
			clientFifo = open(clientFifoName,O_WRONLY);
			clientFifoNum = open(clientFifoName2,O_WRONLY);
			r_read(mainFifofd2, buf, BUFSIZE); 
		
			r_write(clientFifo,&pid,sizeof(pid_t)); 
			r_write(clientFifoNum,&processSize,sizeof(int)); 
		    // Print the read string and close 
		   
 			//pipe communication
		    close(fd[0]);
		    sprintf(bufout, "%d\t", buf[0]);
		    
		    write(fd[1], bufout, strlen(bufout)+1);
		    close(fd[1]);

			//printf("Musteri [%ld] %d lira aldi ",(long)pid,buf[0]);
		    r_close(mainFifofd2); 
			r_close(clientFifo); 
	  		r_close(clientFifoNum);
			unlink(serverFifoName);
			unlink(clientFifoName);
			unlink(clientFifoName2);
			exit(0);
		}
		//parent
		else{
			close(fd[1]);
			bytesin = read(fd[0], bufin, BUFSIZE);
			fprintf(stderr, "Musteri[%ld] %s lira aldi\n",(long)pid, bufin); 
			
		}

	}
	while (r_wait(NULL)) ;
	
	return 0;
}


/* Private functions */

static int gettimeout(struct timeval end,
                               struct timeval *timeoutp) {
   gettimeofday(timeoutp, NULL);
   timeoutp->tv_sec = end.tv_sec - timeoutp->tv_sec;
   timeoutp->tv_usec = end.tv_usec - timeoutp->tv_usec;
   if (timeoutp->tv_usec >= MILLION) {
      timeoutp->tv_sec++;
      timeoutp->tv_usec -= MILLION;
   }
   if (timeoutp->tv_usec < 0) {
      timeoutp->tv_sec--;
      timeoutp->tv_usec += MILLION;
   }
   if ((timeoutp->tv_sec < 0) ||
       ((timeoutp->tv_sec == 0) && (timeoutp->tv_usec == 0))) {
      errno = ETIME;
      return -1;
   }
   return 0;
}
/*tüm sinyal kontrollerinin yapildigi fonksiyon*/
void signal_handler_client(int signumber)
{
	if(signumber == SIGINT)
	{
		fprintf(stdout, "Client has got a SIGINT signal!\n");
	}

	else if(signumber == SIGTERM)
	{
		fprintf(stdout, "Client[%ld] has got a SIGTERM signal!\n", (long)getpid());	
	}

	else if(signumber == SIGUSR1)
	{		
		fprintf(stdout, "Client has got a SIGUSR1 signal!\n");		
	}

	else if(signumber == SIGUSR2)
	{	
		fprintf(stdout, "Client has got a SIGUSR2 signal!\n");		
	}

	else if(signumber == SIGQUIT)
	{
		fprintf(stdout, "Client has got a SIGQUIT signal!\n");
	}

	else if(signumber == SIGFPE)
	{
		fprintf(stdout, "Client has got a SIGFPE signal!\n");	
	}

	else if(signumber == SIGALRM)
	{
		fprintf(stdout, "Client has got a SIGALRM signal!\n");
	}

	kill(pid, SIGUSR1);
	exit(signumber);
}
//////////////NOT////////////////////////////////////////////////
/*Bu fonksiyonlar kitaptaki restart kütüphanesinden alinmistir.*/
/* Restart versions of traditional functions */

int r_close(int fildes) {
   int retval;
   while (retval = close(fildes), retval == -1 && errno == EINTR) ;
   return retval;
}

int r_dup2(int fildes, int fildes2) {
   int retval;
   while (retval = dup2(fildes, fildes2), retval == -1 && errno == EINTR) ;
   return retval;
}


int r_open2(const char *path, int oflag) {
   int retval;
   while (retval = open(path, oflag), retval == -1 && errno == EINTR) ;
   return retval;
}

int r_open3(const char *path, int oflag, mode_t mode) {
   int retval;
   while (retval = open(path, oflag, mode), retval == -1 && errno == EINTR) ;
   return retval;
}

ssize_t r_read(int fd, void *buf, size_t size) {
   ssize_t retval;
   while (retval = read(fd, buf, size), retval == -1 && errno == EINTR) ;
   return retval;
}

pid_t r_wait(int *stat_loc) {
   pid_t retval;
   while (((retval = wait(stat_loc)) == -1) && (errno == EINTR)) ;
   return retval;
}

pid_t r_waitpid(pid_t pid, int *stat_loc, int options) {
   pid_t retval;
   while (((retval = waitpid(pid, stat_loc, options)) == -1) &&
           (errno == EINTR)) ;
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

struct timeval add2currenttime(double seconds) {
   struct timeval newtime;

   gettimeofday(&newtime, NULL);
   newtime.tv_sec += (int)seconds;
   newtime.tv_usec += (int)((seconds - (int)seconds)*D_MILLION + 0.5);
   if (newtime.tv_usec >= MILLION) {
      newtime.tv_sec++;
      newtime.tv_usec -= MILLION;
   }
   return newtime;
}

int copyfile(int fromfd, int tofd) {
   int bytesread;
   int totalbytes = 0;

   while ((bytesread = readwrite(fromfd, tofd)) > 0)
      totalbytes += bytesread;
   return totalbytes;
}

ssize_t readblock(int fd, void *buf, size_t size) {
   char *bufp;
   ssize_t bytesread;
   size_t bytestoread;
   size_t totalbytes;
 
   for (bufp = buf, bytestoread = size, totalbytes = 0;
        bytestoread > 0;
        bufp += bytesread, bytestoread -= bytesread) {
      bytesread = read(fd, bufp, bytestoread);
      if ((bytesread == 0) && (totalbytes == 0))
         return 0;
      if (bytesread == 0) {
         errno = EINVAL;
         return -1;
      }  
      if ((bytesread) == -1 && (errno != EINTR))
         return -1;
      if (bytesread == -1)
         bytesread = 0;
      totalbytes += bytesread;
   }
   return totalbytes;
}

int readline(int fd, char *buf, int nbytes) {
   int numread = 0;
   int returnval;

   while (numread < nbytes - 1) {
      returnval = read(fd, buf + numread, 1);
      if ((returnval == -1) && (errno == EINTR))
         continue;
      if ((returnval == 0) && (numread == 0))
         return 0;
      if (returnval == 0)
         break;
      if (returnval == -1)
         return -1;
      numread++;
      if (buf[numread-1] == '\n') {
         buf[numread] = '\0';
         return numread;
      }  
   }   
   errno = EINVAL;
   return -1;
}

ssize_t readtimed(int fd, void *buf, size_t nbyte, double seconds) {
   struct timeval timedone;

   timedone = add2currenttime(seconds);
   if (waitfdtimed(fd, timedone) == -1)
      return (ssize_t)(-1);
   return r_read(fd, buf, nbyte);
}

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

int readwriteblock(int fromfd, int tofd, char *buf, int size) {
   int bytesread;

   bytesread = readblock(fromfd, buf, size);
   if (bytesread != size)         /* can only be 0 or -1 */
      return bytesread;
   return r_write(tofd, buf, size);
}

int waitfdtimed(int fd, struct timeval end) {
   fd_set readset;
   int retval;
   struct timeval timeout;
 
   if ((fd < 0) || (fd >= FD_SETSIZE)) {
      errno = EINVAL;
      return -1;
   }  
   FD_ZERO(&readset);
   FD_SET(fd, &readset);
   if (gettimeout(end, &timeout) == -1)
      return -1;
   while (((retval = select(fd+1, &readset, NULL, NULL, &timeout)) == -1)
           && (errno == EINTR)) {
      if (gettimeout(end, &timeout) == -1)
         return -1;
      FD_ZERO(&readset);
      FD_SET(fd, &readset);
   }
   if (retval == 0) {
      errno = ETIME;
      return -1;
   }
   if (retval == -1)
      return -1;
   return 0;
}
