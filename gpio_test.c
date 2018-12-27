#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char *argv[])
{
	char wrbuf[5], rdbuf[5];
	int fd, ret,ret1;
	
 
	if(argc!= 3)
	{
		printf("%s <device path>\n", argv[0]);
		_exit(1);
	}
	
	fd = open(argv[1], O_WRONLY);
	if(fd < 0)
	{
		perror("open() failed");
		_exit(1);
	}
	
	strcpy(wrbuf,argv[2]);
	printf("USER %s | %d\n",wrbuf,strlen(wrbuf));
	//getchar();
	ret = write(fd, wrbuf, strlen(wrbuf));
	if(ret < 0)
	{
		printf("Write Failed()\n");
		close(fd);
		exit(0);	
	}		
	printf("Buf Written(%d) : %s\n", ret, wrbuf);
	
	printf("Closing file\n");
	close(fd);
	printf("Closed file\n");

	return 0;
}


