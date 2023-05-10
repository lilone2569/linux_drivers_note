#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>




int main(int argc,char *argv[])
{
	int fd = 0;
	int ret = 0;
	char read_buf[100] = {0};
	char write_buf[100] = {0};
	char user_data[] = "user data";
	if(argc != 3)
	{
		printf("error usage!\r\n");
	}
		
	

	fd = open(argv[1],O_RDWR);
	if(fd < 0)
	{
		printf("open file failed!\r\n");
		return 0;
	}
	if(atoi(argv[2]) == 1)  //read chrdevbase
	{
		ret = read(fd,read_buf,sizeof(read_buf));
		if(ret < 0)
		{
			printf("read file failed!\r\n");
		}
		else 
		{
			printf("read_buf:%s\r\n",read_buf);
		}
	}

	

	else if(atoi(argv[2]) == 2)  //write chrdevbase
	{
		memcpy(write_buf,user_data,sizeof(user_data));
		ret = write(fd,write_buf,sizeof(write_buf));
		if(ret < 0)
		{
			printf("write file failed!\r\n");
		}
	}
	
	ret = close(fd);
	if(ret < 0) printf("close file failed!\r\n");
	



	return 0;
}	

