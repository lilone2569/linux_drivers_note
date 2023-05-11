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
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/ioctl.h>
#include <linux/irq_work.h>
#include <linux/workqueue.h>



#define LED_OFF 0
#define LED_ON 1
#define GPIOLED_NAME "led"

#define OPEN_CMD        _IO(0xef,1)      //open timer
#define CLOSE_CMD       _IO(0xef,2)      //close timer
#define SETPERIOD_CMD   _IO(0xef,3)      //set timer period 




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
    struct timer_list timer;
};
#define OPEN_CMD        _IO(0xef,1)
#define CLOSE_CMD       _IO(0xef,2)
#define SETPERIOD_CMD   _IO(0xef,3)






struct gpioled_dev gpioled; /* led 设备 */



static void timer_function(unsigned long arg)
{
    struct gpioled_dev *dev = (struct gpioled_dev*)arg;
    static int sta = 1;
    sta = !sta;
    
    gpio_set_value(dev->led_gpio,sta);

    mod_timer(&dev->timer,jiffies + msecs_to_jiffies(500));

}



int led_open(struct inode *node, struct file *file)
{
    file->private_data = &gpioled;

    return 0;
}





int led_release(struct inode *node, struct file *file)
{
    //

    return 0;
}

long timer_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int value;
    switch(cmd)
    {
        case OPEN_CMD:    
            mod_timer(&gpioled.timer,jiffies + msecs_to_jiffies(500));
            break;
        case CLOSE_CMD:    
            del_timer_sync(&gpioled.timer);
            break;
        case SETPERIOD_CMD:    
            value = arg;
            mod_timer(&gpioled.timer,jiffies + msecs_to_jiffies(value));
            break;
        default:
            break;
    }

    return 0;
}




static struct file_operations f_ops = 
{
    .owner = THIS_MODULE,
    .open = led_open,
    .unlocked_ioctl = timer_ioctl,
    .release = led_release
};


static int __init led_init(void)
{
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


    gpioled.timer.function = timer_function;
    gpioled.timer.expires = jiffies + msecs_to_jiffies(500);
    gpioled.timer.data = (unsigned long)&gpioled;
    init_timer(&gpioled.timer);

    add_timer(&gpioled.timer);

    return 0;
}


static int __exit led_exit(void)
{
    gpio_set_value(gpioled.led_gpio, 1);
    del_timer(&gpioled.timer);
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



