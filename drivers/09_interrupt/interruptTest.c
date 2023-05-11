#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>


#define OPEN_CMD        _IO(0xef,1)
#define CLOSE_CMD       _IO(0xef,2)
#define SETPERIOD_CMD   _IO(0xef,3)


int main(int argc,char *argv[])
{
    int fd = 0;
    int ret = 0;
    unsigned char str;
    unsigned int cmd;
    unsigned int arg;
    unsigned char databuf[1];
    if(argc != 2)
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

    while(1)
    {
        printf("please input cmd:\r\n");
        ret = scanf("%d",&cmd);
        if(ret != 1)
        {
            gets(str);
        }
        if(cmd == 1)
        {
            cmd = OPEN_CMD;
            printf("open timer\r\n");
        }
        else if(cmd == 2)
        {
            cmd = CLOSE_CMD;
            printf("close timer\r\n");
        }
        else if(cmd == 3)
        {
            cmd = SETPERIOD_CMD;
            printf("input period:\r\n");
            ret = scanf("%d",&arg);
            if(ret != 1)
            {
                gets(str);
            }
        }

        ioctl(fd,cmd,arg);
    }
    


    

    ret = close(fd);
    if(ret < 0)
    {
        printf("close file failed!\r\n");
    }
    
    
    return 0;
}












