/*******************************/
/*      131044055              */
/*    Eda BAHRIOGLU            */
/*******************************/

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

/*Static value*/
static int sizeFile = 0;

/*Program Fonksiyonlari*/
int postOrderApply (char *path, int pathfun (char *path1));
int sizepathfun (char *path);

/*Dosya kontrollerini iceren Fonksiyonlar*/
int is_directory(char *Path);
int is_regular(char *Path);
int is_nonRegular(char *Path);

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
		printf("Total File Size: %d\n", sizeFile);
    }

	/*Tüm directory lerin sizeof toplamı için girisi saglayan komut satiri*/
	if(argc== 3)
	{
		if(strcmp(argv[1],"-z")==0)
		{
			size = postOrderApply(argv[2],sizepathfun);
			printf("Total Directory Size: %d\n", size);
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
		        dir_size = postOrderApply(filePath,pathfun) + pathfun(filePath);
		        printf("SIZE: %d\t", dir_size);
		        printf("PATH: %s\n", filePath);
		        dirSizeOf += dir_size;
		    }

		    else if(is_regular(filePath)==1)
		    {
		        dirSizeOf += pathfun(filePath);
		        printf("SIZE: %d\t", pathfun(filePath));
				sizeFile += pathfun(filePath);
		        printf("PATH: %s\n", filePath);
		    }

			else if(is_nonRegular(filePath)==1)
			{
				printf("PATH: %s\n", filePath);
				printf("This is a Special File\n");
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


