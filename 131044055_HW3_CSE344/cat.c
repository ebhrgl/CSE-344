#include <stdio.h>
#include <stdlib.h>
            
int main(int argc, char const *argv[])
{
	FILE *file;
	char str[10000];

	if(argc<2)
	{
		fprintf(stderr, "Unknown number of inputs.\n");
		exit(-1);
	}
	else
	{
		//file open
		if((file = fopen(argv[1],"r"))!=NULL)
		{
			// reading file line by line 
			while(fscanf(file,"%[^\n]\n",str)!=EOF)
			{
				printf("%s\n", str);
			}
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
