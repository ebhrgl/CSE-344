#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 

int main(int argc, char const *argv[])
{
	FILE * file;
	int lineCount=0;
	char c;

	if(argc<2)
	{	
		fprintf(stderr, "Unknown number of inputs.\n");
		return -1;
	}
	else
	{
		//file open
		if((file = fopen(argv[1],"r"))!=NULL)
		{
			while((c=getc(file))!=EOF)
			{
				if(c=='\n'){
				lineCount++;
				}
			}
			printf("%d\n",lineCount );
			//file close
			fclose(file);
		}
		else
		{	
			fprintf(stderr,"cat: %s: No such file or directory\n",argv[1]);
		}
	}
	exit(EXIT_SUCCESS);
}

