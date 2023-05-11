#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>


#define KEY_VAL 0xf0


int main(int argc,char *argv[])
{
    int fd;
    unsigned char buf[5];
    char key_val;
    if(argc != 2)
    {
        printf("usage error!\r\n");
        return -1;
    }
    fd = open(argv[1],O_RDWR);
    if(fd < 0)
    {
        printf("open file failed!\r\n");
        return -1;
    }
    
    while(1)
    {
        read(fd,&key_val,1);
        if(key_val == KEY_VAL)
        {
            printf("key press! value = %x\r\n",key_val);
        }
    }


    return 0;
}






