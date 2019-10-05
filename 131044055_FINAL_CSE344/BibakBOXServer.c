#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <stdio.h>
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
#define MAX 80 
#define PORT 8080 
#define SA struct sockaddr 
#define BLKSIZE PIPE_BUF
#define BLUE_TEXT(x) "\033[34;1m" x "\033[0m"
struct client_t
{
	char pathNameDest[1024];
	char pathNameSource[1024];
	int size;
};

FILE *logfile;

struct client_t clients;
int portNum=0;
int threadpoolSize=0;
pid_t client_pid;
char temp[1024];
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


void server_sinyal_handler(int signumber)
{
	if(signumber == SIGINT)
	{
		fprintf(logfile, "<CTRL-C> occured for the server!\n");
		fprintf(stdout, "Server has got a SIGINT signal!\n");
		kill(client_pid,SIGINT);
	}
					
	else if(signumber == SIGTERM)
	{
		fprintf(logfile, "Client stopped respoding!\n");
		fprintf(stdout, "Server[%ld] has got a SIGTERM signal!\n", (long)getpid());
		kill(client_pid,SIGTERM);	
	}
		
	else if(signumber == SIGQUIT)
	{
		
		fprintf(logfile, "SIGQUIT signal. Server will be r_closed.\n");
		fprintf(stdout, "SErver has got a SIGQUIT signal!\n");	
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
// Driver function 
int main(int argc, char *argv[]) 
{ 
	struct timeval begin,end;
    int sockfd, connfd, len; 
    struct sockaddr_in servaddr, cli; 
  	int y,n,x;
	int k;
	double totalTime;
	struct sigaction act;
	
	if (argc != 4) {
		fprintf(stderr, "Usage: %s\n", argv[0]);
        fprintf(stderr, "\n<Directory>");
        fprintf(stderr, "\n<threadpoolSize>");
        fprintf(stderr, "\n<Portnumber>");
        exit(EXIT_FAILURE);
	}

	srand(time(NULL));

	gettimeofday(&begin, NULL);
	logfile=fopen("Server.log","w");
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
	

	portNum = atoi(argv[3]);
	strcpy(clients.pathNameDest,argv[1]);
    // socket create and verification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n");
		fprintf(logfile, "socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created..\n"); 
		fprintf(logfile,"Socket successfully created..\n"); 
   		bzero(&servaddr, sizeof(servaddr)); 
  
    // assign IP, PORT 
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(portNum);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  
    // Binding newly created socket to given IP and verification 
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
        printf("socket bind failed...\n"); 
		fprintf(stderr, BLUE_TEXT("\nFileShareServer : ") "This port is used!\n\n");
		fprintf(logfile, BLUE_TEXT("\nFileShareServer : ") "This port is used!\n\n");
		fprintf(logfile,"socket bind failed...\n"); 
        exit(0); 
    } 

 
    else
        printf("Socket successfully binded..\n"); 
  
    // Now server is ready to listen and verification 
    if ((listen(sockfd, 5)) != 0) { 
        printf("Listen failed...\n"); 
		fprintf(logfile,"Listen failed...\n");
        exit(0); 
    } 
    else
        printf("Server listening..\n"); 
		fprintf(logfile,"Server listening..\n"); 
    	len = sizeof(cli); 
  
	while(1){
			// Accept the data packet from client and verification 
			connfd = accept(sockfd, (SA*)&cli, &len); 
			if (connfd < 0) { 
				printf("server acccept failed...\n");
				exit(0); 
			} 
			else

			printf("server acccept the client...\n"); 
			fprintf(logfile,"server acccept the client...\n"); 
			strcpy(clients.pathNameDest,argv[1]);
			r_write(connfd,&clients.pathNameDest,sizeof(char)*1024);
	
			//printf(" eeeeee2   %s\n",clients.pathNameSource);
			threadpoolSize = atoi(argv[2]);
			clients.size = threadpoolSize;
			r_write(connfd,&clients.size,sizeof(clients.size));
			r_read(connfd,clients.pathNameSource, sizeof(char)*1024);
			r_read(connfd,&client_pid, sizeof(pid_t));
			printf("CLIENT ID   %d\n",client_pid);
			fprintf(logfile,"CLIENT ID   %d\n",client_pid);
			//printf(" eeeeee2   %s\n",clients.pathNameSource);
			gettimeofday(&end, NULL);
			totalTime= (double) (end.tv_usec - begin.tv_usec) / 1000000 +(double) (end.tv_sec - begin.tv_sec);
			printf("total time spent %.3f seconds.\n",totalTime);
			fprintf(logfile,"total time spent %.3f seconds.\n",totalTime);
			sprintf(temp,"%s/%s",argv[1],clients.pathNameSource);
			mkdir(temp,0777);
			chdir(argv[1]);
		}
		
		close(sockfd); 
		fclose(logfile);
} 
