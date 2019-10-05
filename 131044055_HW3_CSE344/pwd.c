#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#define MAX_SIZE 1024

int main(int argc, char const *argv[])
{
	char path[MAX_SIZE];
    if (getcwd(path, sizeof(path)) != NULL)
        printf("%s\n", path);
    else
        perror("error");

	exit(EXIT_SUCCESS);
}

