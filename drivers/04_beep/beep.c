#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/fs.h>



#define DEVICE_NAME "beep"
#define BEEP_OFF 0
#define BEEP_ON 1


struct gpiobeep {
    int major;
    int minor;
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    int beep_gpio;
    struct device_node *node;
};

struct gpiobeep dev_beep;


int beep_open(struct inode *node, struct file *file)
{
    file->private_data = &dev_beep;

    return 0;
}
ssize_t beep_write(struct file *file, const char __user *buf, size_t cnt, loff_t *offset)
{
    int ret;
    unsigned char databuf[1];
    unsigned char beep_status;
    struct gpiobeep *dev = file->private_data;
    ret = copy_from_user(databuf,buf,1);
    if(ret < 0)
    {
        printk("copy from user failed!\r\n");
        return -1;
    }
    beep_status = databuf[0];
    if(beep_status == BEEP_OFF)
    {
        gpio_set_value(dev->beep_gpio,1);  //high level OFF
    }
    else if(beep_status == BEEP_ON)
    {
        gpio_set_value(dev->beep_gpio,0);  //low level ON
    }

    return 0;
}



int beep_release(struct inode *node, struct file *file)
{

    return 0;
}






struct file_operations f_ps = {
    .owner = THIS_MODULE,
    .open = beep_open,
    .write = beep_write,
    .release = beep_release
};



static int __init beep_init(void)
{
    int ret;
    dev_beep.node = of_find_node_by_path("/gpiobeep");
    if(dev_beep.node == NULL)
    {
        printk("find node path failed!\r\n");
        return -1;
    }
    dev_beep.beep_gpio = of_get_named_gpio(dev_beep.node,"led-gpio",0);
    if(dev_beep.beep_gpio < 0)
    {
        printk("get gpio failed!\r\n");
        return -1;
    }
    ret = gpio_direction_output(dev_beep.beep_gpio,1); 
    if(ret < 0)
    {
        printk("set beep gpio default output failed!\r\n");
        return -1;
    }
    /* 注册字符设备驱动 */
    if(dev_beep.major)
    {
        dev_beep.devid = MKDEV(dev_beep.major,0);  //create device id
        register_chrdev_region(dev_beep.devid,1,DEVICE_NAME);  //register chr decvice
    }
    else
    {
        alloc_chrdev_region(&dev_beep.devid,0, 1,DEVICE_NAME);
        dev_beep.major = MAJOR(dev_beep.devid);
        dev_beep.minor = MINOR(dev_beep.devid);
    }
    printk("major=%d,minor=%d",dev_beep.major,dev_beep.minor);
    cdev_init(&dev_beep.cdev,&f_ps);
    cdev_add(&dev_beep.cdev,dev_beep.devid,1);
    dev_beep.class = class_create(THIS_MODULE,DEVICE_NAME);
    if(dev_beep.class == NULL)
    {
        printk("class create failed!\r\n");
        return -1;
    }
    dev_beep.device = device_create(dev_beep.class,NULL,dev_beep.devid,NULL,DEVICE_NAME);
    if(dev_beep.device == NULL)
    {
        printk("device create failed!\r\n");
        return -1;
    }


    return 0;
}


static int __exit beep_exit(void)
{
    unregister_chrdev_region(dev_beep.devid,1); /* 注销 */
    cdev_del(&dev_beep.cdev);
    class_destroy(dev_beep.class); 
    device_destroy(dev_beep.class,dev_beep.devid);
    return 0;
}





module_init(beep_init);
module_exit(beep_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lilone");


