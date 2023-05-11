#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc,char *argv[])
{
    int cnt = 0;
    int fd = 0;
    int ret = 0;
    unsigned char databuf[1];
    if(argc != 3)
    {
        printf("usage error!");
        return 0;
    }

    fd = open(argv[1],O_RDWR);
    if(fd < 0)
    {
        printf("open file failed!\r\n");
        return 0;
    }
    databuf[0] = atoi(argv[2]);
    ret = write(fd,databuf,1);
    if(ret < 0)
    {
        printf("control led failed!\r\n");
    }

    while(1)
    {
        sleep(5);
        cnt++;
        printf("led running times = %d\r\n",cnt);
        if(cnt >= 5) break;
    }


    ret = close(fd);
    if(ret < 0)
    {
        printf("close file failed!\r\n");
    }

    
    return 0;
}












