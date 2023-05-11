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
#include <linux/semaphore.h>




#define kEY_VALUE 0xf0
#define INVALIB_VALUE 0x00

#define DEVICE_NAME "key"

struct device_key {
    int major;
    int minor;
    dev_t devid;
    int key_gpio;
    atomic_t key_val;
    struct cdev cdev;
    struct class *class;
    struct device_node *node;
    struct device *device;
};

struct device_key key;

static void key_io_init(void)
{
    key.node = of_find_node_by_path("/gpiokey");
    if(key.node == NULL)
    {
        printk("get node by path failed!\r\n");
        return -1;
    }
    key.key_gpio = of_get_named_gpio(key.node,"key-gpio",0);
    printk("key gpio=%d\r\n",key.key_gpio);
    gpio_request(key.key_gpio,"key0");
    gpio_direction_input(key.key_gpio);
}


int key_open(struct inode *node, struct file *file)
{
    file->private_data = &key;
    key_io_init();

    return 0;
}

ssize_t key_read(struct file *file, char __user *buf, size_t cnt, loff_t *offset)
{
    int ret;
    unsigned char val;
    struct device_key *dev = file->private_data;
    if(gpio_get_value(dev->key_gpio) == 0)
    {
        while(!gpio_get_value(dev->key_gpio));
        atomic_set(&dev->key_val,kEY_VALUE);
    }
    else 
    {
        atomic_set(&dev->key_val,INVALIB_VALUE);
    }
    val = atomic_read(&dev->key_val);
    ret = copy_to_user(buf,&val,1);
    return ret;
}

struct file_operations fps = {
    .owner = THIS_MODULE,
    .open = key_open,
    .read = key_read
};



static int __init key_init(void)
{
    

    atomic_set(&key.key_val,INVALIB_VALUE);

    /* 注册字符设备驱动 */
    if(key.major)
    {
        key.devid = MKDEV(key.major,0);  //create device id
        register_chrdev_region(key.devid,1,DEVICE_NAME);  //register chr decvice
    }
    else
    {
        alloc_chrdev_region(&key.devid,0, 1,DEVICE_NAME);
        key.major = MAJOR(key.devid);
        key.minor = MINOR(key.devid);
    }
    printk("major=%d,minor=%d",key.major,key.minor);
    key.cdev.owner = THIS_MODULE;
    cdev_init(&key.cdev,&fps);
    cdev_add(&key.cdev,key.devid,1);
    key.class = class_create(THIS_MODULE,DEVICE_NAME);
    if(key.class == NULL)
    {
        printk("class create failed!\r\n");
        return -1;
    }
    key.device = device_create(key.class,NULL,key.devid,NULL,DEVICE_NAME);
    if(key.device == NULL)
    {
        printk("device create failed!\r\n");
        return -1;
    }


    return 0;
}


static int __exit key_exit(void)
{
    unregister_chrdev_region(key.devid,1); /* 注销 */
    cdev_del(&key.cdev);
    class_destroy(key.class); 
    device_destroy(key.class,key.devid);
    return 0;
}


module_init(key_init);
module_exit(key_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lilone");



