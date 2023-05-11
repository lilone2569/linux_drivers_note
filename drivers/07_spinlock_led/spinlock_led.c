//测试app与led一样

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






#define LED_OFF 0
#define LED_ON 1
#define GPIOLED_NAME "led"




/* gpioled 设备结构体 */
struct gpioled_dev{
    dev_t devid; /* 设备号 */
    struct cdev cdev; /* cdev */
    struct class *class; /* 类 */
    struct device *device; /* 设备 */
    int major; /* 主设备号 */
    int minor; /* 次设备号 */
    struct device_node *nd; /* 设备节点 */
    int led_gpio; /* led 所使用的 GPIO 编号 */
    //atomic_t lock;
    spinlock_t lock;
    int status;

    
};


struct gpioled_dev gpioled; /* led 设备 */


int led_open(struct inode *node, struct file *file)
{
    
    file->private_data = &gpioled;
    /*if(!atomic_dec_and_test(&gpioled.lock))
    {
        atomic_inc(&gpioled.lock);
        return -EBUSY;
    }*/
    spin_lock(&gpioled.lock);
    if(gpioled.status)
    {
        spin_unlock(&gpioled.lock);
        return -EBUSY;
    }
    gpioled.status++;
    spin_unlock(&gpioled.lock);


    return 0;
}


ssize_t led_write(struct file *file, const char __user *buf, size_t cnt, loff_t *offset)
{
    unsigned char led_status;
    unsigned char databuf[1];
    struct gpioled_dev *dev = file->private_data;


    int ret = copy_from_user(databuf,buf,1);
    if(ret < 0)
    {
        printk("kernel write failed!\r\n");
        return -1;
    }
    led_status = databuf[0];
    if(led_status == LED_OFF)
    {
        gpio_set_value(dev->led_gpio,1);
    }
    else if(led_status == LED_ON)
    {
        gpio_set_value(dev->led_gpio,0);
    }

    return 0;
}


int led_release(struct inode *node, struct file *file)
{
    
    //atomic_inc(&gpioled.lock);
    spin_lock(&gpioled.lock);
    if(gpioled.status)
    {
        gpioled.status--;
    }
    
    spin_unlock(&gpioled.lock);


    return 0;

}


static struct file_operations f_ops = 
{
    .owner = THIS_MODULE,
    .open = led_open,
    .write = led_write,
    .release = led_release
};


static int __init led_init(void)
{
    //atomic_set(&gpioled.lock,1);
    spin_lock_init(&gpioled.lock);

    gpioled.status = 0;   //status = 0:未使用led
    int ret;
    gpioled.nd = of_find_node_by_path("/gpioled");
    if(gpioled.nd == NULL)
    {
        printk("gpioled node not found!\r\n");
        return -1;
    }
    else 
    {
        printk("gpioled node found=%s",gpioled.nd->name);
    }
    gpioled.led_gpio = of_get_named_gpio(gpioled.nd,"led-gpio",0);
    if(gpioled.led_gpio < 0)
    {
        printk("get led-gpio failed!\r\n");
        return -1;
    }
    printk("led-gpio=%d",gpioled.led_gpio);

    ret = gpio_direction_output(gpioled.led_gpio,1); 
    if(ret < 0)
    {
        printk("set gpio output failed!\r\n");
        return -1;
    }

    /* 注册字符设备驱动 */
/* 1、创建设备号 */
    if (gpioled.major) { /* 定义了设备号 */
        gpioled.devid = MKDEV(gpioled.major, 0);
        register_chrdev_region(gpioled.devid, 1,
        GPIOLED_NAME);
    } else { /* 没有定义设备号 */
        alloc_chrdev_region(&gpioled.devid, 0, 1,
        GPIOLED_NAME); /* 申请设备号 */
        gpioled.major = MAJOR(gpioled.devid); /* 获取分配号的主设备号 */
        gpioled.minor = MINOR(gpioled.devid); /* 获取分配号的次设备号 */
    }
    printk("gpioled major=%d,minor=%d\r\n",gpioled.major,gpioled.minor);

    cdev_init(&gpioled.cdev, &f_ops);
    cdev_add(&gpioled.cdev, gpioled.devid, 1);
    gpioled.class = class_create(THIS_MODULE, GPIOLED_NAME);
    if(IS_ERR(gpioled.class)) {
        return PTR_ERR(gpioled.class);
    }
    gpioled.device = device_create(gpioled.class, NULL,gpioled.devid, NULL, GPIOLED_NAME);
    if(IS_ERR(gpioled.device)) {
        return PTR_ERR(gpioled.class);
    }
    return 0;
}


static int __exit led_exit(void)
{

    /* 注销字符设备驱动 */
    cdev_del(&gpioled.cdev); /* 删除 cdev */
    unregister_chrdev_region(gpioled.devid, 1); /* 注销 */

    device_destroy(gpioled.class, gpioled.devid);
    class_destroy(gpioled.class);
    return 0;
}


module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lilone");



