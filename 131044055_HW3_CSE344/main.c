/*******************************/
/*      131044055              */
/*    Eda BAHRIOGLU            */
/*       hw3                   */
/*******************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

static char prevArray[1024][1024];
int cd(char *directName);
void help();
void exitFunc();
char **parseFunc(char *command);
void operation(char** array);
char *readLine(void);
void handler(int signo);
void previous(char**array);
int main(int argc, char* argv[]){

	int status;
	struct sigaction sa;
	char **commandList;
	int bufsize = 1024;
	char character;                              
	int   i;        
	char* line = malloc(sizeof(char) * bufsize);                                       
	i = 0; 
	 
    memset(&sa,0, sizeof(sa));
    sa.sa_handler=&handler;
    sigaction(SIGINT,&sa,NULL);
    sigset_t proc,prevProc;
    sigemptyset(&proc);
    sigaddset(&proc,SIGINT); 
       

	if( (sigemptyset(&sa.sa_mask)== -1) || (sigaction(SIGINT, &sa, NULL) == -1 ) || (sigaction(SIGTERM, &sa, NULL) == -1)) {
		perror("Failed to install Signal handlers");
		exit(-1);
	}
	
	printf("WELCOME TO SHELL PROGRAM\n");
	printf("ENTER COMMAND\n");
	while(1)
	{	
        line = readLine();
		commandList = parseFunc(line);	
		sigprocmask(SIG_BLOCK,&proc,&prevProc);
		operation(commandList);
		sigprocmask(SIG_SETMASK,&prevProc,NULL);
		free(line);
        free(commandList);			
	}
	return 0;

}

//signal control
void handler(int signo){
    fprintf(stdout,"!SIGNAL HANDLER.\n");
    exit(0);
}

//read from keyword
char *readLine(void)
{
    int bufsize = 1024;
    int i = 0;
    char *line = malloc(sizeof(char) * bufsize);
    int character;

    while (1) {
        character = getchar();
		line[i] = character;   
       
        if (character == '\n' || character == EOF ) {
            line[i] = '\0';
            return line;
        }
           
		i++;
    }
}

//directory change 
int cd(char *directName) {
	if (chdir(directName) == -1) {
		perror("No such file or directory.\n");
	}
	getcwd(directName, sizeof(directName));
	return 1;
}

//line split func
char **parseFunc(char *command)
{
	int index = 0;
	char* token;
	int j;
	char **commandTokens = malloc(sizeof(char*)*1024);
	token = strtok(command, " \n\t");

	while(token != NULL)
	{		
		//printf("%s\n",token);
		commandTokens[index] = token;
		index++;
		token = strtok(NULL," \n\t" );
	}

	
	commandTokens[index] = NULL;
	return commandTokens;
 	
}

void previous(char**array) {
	int i = 0;
	while(array[i] != NULL) {	
		printf("%d- %s\n", atoi(array[i+1]), prevArray[atoi(array[i+1])]);
		++i;
	}
}

//All operations
void operation(char** array)
{
 	pid_t fork_pid, waitpidler;
	int fileDescriptor;
	int pipeFd[2];
	int status;
	int j;
	
	
    if (strcmp(array[0],"cd") == 0)
	{
         cd(array[1]);
	}
		
    else if (strcmp(array[0],"help") == 0)
	{
         help();
	}
	
	else if(strcmp(array[0],"!") == 0)
	{
		previous(array);
	}
    else if (strcmp(array[0],"exit") == 0)
	{
        exitFunc();
	}

	
	if (strcmp(array[0],"cat") == 0){
		fork_pid = fork();

		if (fork_pid == 0) 
		{
     		execvp("./cat",array);
		}
		else
		{
			while (!WIFEXITED(status) && !WIFSIGNALED(status))
			{
				 wait(&fork_pid);
			}
		}
	}    
	else if (strcmp(array[0],"wc") == 0){
		fork_pid = fork();

		if (fork_pid == 0) 
		{
     		execvp("./wc",array);
		}
		else
		{
			while (!WIFEXITED(status) && !WIFSIGNALED(status))
			{
				 wait(&fork_pid);
			}
		}		    
	}
	else if (strcmp(array[0],"lsf") == 0){
		fork_pid = fork();

		if (fork_pid == 0) 
		{
     		execvp("./lsf",array);
		}
		else
		{
			while (!WIFEXITED(status) && !WIFSIGNALED(status))
			{
				 wait(&fork_pid);
			}
		}
	}

	else if (strcmp(array[0],"pwd") == 0){
		fork_pid = fork();

		if (fork_pid == 0) 
		{
     		execvp("./pwd",array);
		}
		else
		{
			while (!WIFEXITED(status) && !WIFSIGNALED(status))
			{
				 wait(&fork_pid);
			}
		}
	}

	else if (strcmp(array[0],"du") == 0){
		fork_pid = fork();

		if (fork_pid == 0) 
		{
     		execvp("./du",array);
		}
		else
		{
			while (!WIFEXITED(status) && !WIFSIGNALED(status))
			{
				 wait(&fork_pid);
			}
		}
	} 
	
	else if ((strcmp(array[0],"pwd") == 0) ||  (strcmp(array[0],"cat") == 0) ||  (strcmp(array[0],"wc") == 0) || (strcmp(array[0],"du") == 0) || (strcmp(array[0],"lsf") == 0) && (strcmp(array[1],"|") == 0)){
		pipe(pipeFd);

		fork_pid = fork();

		if (fork_pid == 0) 
		{
			dup2(pipeFd[0], 0);

			// close pipe write mode

			close(pipeFd[1]);

			// execute 

			execvp("./cat", array);
			execvp("./wc", array);
			execvp("./lsf", array);
			execvp("./pwd", array);
			execvp("./du", array);
     		
		}
		else
		{
			dup2(pipeFd[1], 1);

			// close pipe read mode

			close(pipeFd[0]);

			// execute 

			execvp("./cat", array);
			execvp("./wc", array);
			execvp("./lsf", array);
			execvp("./pwd", array);
			execvp("./du", array);
			
		}
	} 
		
}

//shell help command
void help()
{
 	printf ("ls   - list file type, access rights, file size and file name of all files in the present working directory\n");
   	printf ("pwd   - which will print the present working directory\n");
   	printf ("cd   - which will change the present working directory to the location provided as argument\n");
   	printf ("cat   - which will print on standard output the contents of the file provided to it as argument\n");
   	printf ("wc   -which will print on standard output the number of lines in the file provided to it as argument\n");
   	printf ("exit   -which will exit the shell\n");
 	printf("help  -which will print the list of supported commands\n");
  	printf(">  -directional right = into file \n");
    printf("<  -directional left = will get from file \n");
    printf("|  -Pipe operation\n");
}

//shell output command
void exitFunc(){
	printf("Bye\n");
	exit(0);
}
