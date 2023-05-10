#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#define CHRDEVBASE_MAJOR 200
#define CHRDEVBASE_NAME "chrdevbase"


static char readbuf[100];
static char writebuf[100];
static char kerneldata[] = {"kernel data!"};



ssize_t chrdevbase_read(struct file *file, char __user *buf, size_t cnt, loff_t *off_t)
{
    int ret = 0;


    memcpy(readbuf,kerneldata,sizeof(kerneldata));
    ret = copy_to_user(buf,readbuf,cnt);
    if(ret == 0)
    {
        printk("kernel send data success!\r\n");
    }
    else printk("kernel send data failed!\r\n");
    return 0;
}

ssize_t chrdevbase_write(struct file *file, const char __user *buf, size_t cnt, loff_t *off_t)
{
    int ret = 0;
    ret = copy_from_user(writebuf,buf,cnt);
    if(ret == 0)
    {
        printk("kernel rec data success!\r\n");
    }
    else printk("kernel rec data failed!\r\n");
    return 0;
}


int chrdevbase_open(struct inode *inode, struct file *file)
{
    //printk("chrdevbase open!\r\n");
    return 0;
}

int chrdevbase_release(struct inode *inode, struct file *file)
{
    //printk("chrdevbase close!\r\n");
    return 0;
}


static struct file_operations chrdevbase_fops = {
    .owner = THIS_MODULE,
    .open = chrdevbase_open,
    .read = chrdevbase_read,
    .write = chrdevbase_write,
    .release = chrdevbase_release,
};




/* 驱动入口函数 */
static int __init chrdevbase_init(void)
{
	/* 入口函数具体内容 */
	int retvalue = 0;

	/* 注册字符设备驱动 */
	retvalue = register_chrdev(CHRDEVBASE_MAJOR,CHRDEVBASE_NAME, &chrdevbase_fops);
	if(retvalue < 0){
	/* 字符设备注册失败,自行处理 */
    
    }
    printk("chrdevbase_init()");
return 0;
}

/* 驱动出口函数 */
static void __exit chrdevbase_exit(void)
{
	/* 注销字符设备驱动 */
	unregister_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME);
    printk("chrdevbase_exit()");
}




module_init(chrdevbase_init);
module_exit(chrdevbase_exit);



MODULE_LICENSE("GPL");
MODULE_AUTHOR("lilone");

