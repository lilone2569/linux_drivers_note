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
#include <linux/of_irq.h>
#include <linux/irq.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/fs.h>
#include <linux/semaphore.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/poll.h>

#define IMX6U_IRQ_NAME "imx6u_irq"
#define kEY_VALUE 0x01
#define INVALIB_VALUE 0xff

#define DEVICE_NAME "key"

struct irq_key
{
    int irq_num;
    unsigned char value;
    irqreturn_t (*irq_handler_t)(int, void *);
    unsigned char name[10];
};


struct device_key {
    int major;
    int minor;
    dev_t devid;
    int key_gpio;
    atomic_t key_val; /* 有效的按键键值 */
    struct cdev cdev;
    struct class *class;
    struct device_node *node;
    struct device *device;
    struct timer_list timer; /* 定义一个定时器*/
    
    atomic_t releasekey; /* 标记是否完成一次完成的按键*/
    struct irq_key irqkey; /* 按键描述数组 */
    unsigned char curkeynum; /* 当前的按键号 */
};

struct device_key key;

irqreturn_t key_irq_handler(int irq, void *dev_id)
{
    struct device_key *dev = (struct device_key*)dev_id;
    key.curkeynum = 0;
    dev->timer.data = (volatile long)dev_id;
    mod_timer(&dev->timer, jiffies + msecs_to_jiffies(10));

    return IRQ_RETVAL(IRQ_HANDLED);
}

void timer_funtion(unsigned long arg)
{
    int value = 0;
    struct device_key *dev = (struct device_key *)arg;
    struct irq_key *irq;
    value = gpio_get_value(dev->key_gpio);
    if(value == 0)   //按键按下
    {
        atomic_set(&dev->key_val,irq->value);

    }
    else if(value == 1)   //按键释放
    {
        atomic_set(&dev->key_val,irq->value | 0x80);
        atomic_set(&dev->releasekey,1);
    }
}
static int key_io_init(void)
{
    int ret;
    key.node = of_find_node_by_path("/gpiokey");  //通过路径查找节点/gpiokey
    if(key.node == NULL)
    {
        printk("get node by path failed!\r\n");
        return -1;
    }
    key.key_gpio = of_get_named_gpio(key.node,"key-gpio",0);   //通过名字查找gpio号
    printk("key gpio=%d\r\n",key.key_gpio);
    gpio_request(key.key_gpio,"key0");   //申请GPIO
    gpio_direction_input(key.key_gpio);  //配置IO为输入模式

    key.irqkey.irq_num = irq_of_parse_and_map(key.node,0);  //获取中断号

    printk("key gpio=%d,irq num=%d\r\n",key.key_gpio,key.irqkey.irq_num);
    key.irqkey.irq_handler_t = key_irq_handler;
    key.irqkey.value = kEY_VALUE;

    //申请中断
    ret = request_irq(key.irqkey.irq_num,key.irqkey.irq_handler_t,IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,"irq_key",&key);
    if(ret < 0)
    {
        printk("irq request failed!\r\n");
        return -1;
    }
    init_timer(&key.timer);
    key.timer.function = timer_funtion;
    return 0;
}


int key_open(struct inode *node, struct file *file)
{
    file->private_data = &key;
    

    return 0;
}

ssize_t key_read(struct file *file, char __user *buf, size_t cnt, loff_t *offset)
{

    int ret;
    unsigned char val;
    unsigned char release_key;
    struct device_key *dev = (struct device_key *)file->private_data;
    val = atomic_read(&dev->key_val);
    release_key = atomic_read(&dev->releasekey);
    if(release_key)
    {
        if(val & 0x80)
        {
            val &= ~(0x80);
            copy_to_user(buf, &val, sizeof(val));
        }
        else return -EINVAL;
    }
    else return -EINVAL;
    return 0;

}

struct file_operations fps = {
    .owner = THIS_MODULE,
    .open = key_open,
    .read = key_read
};



static int __init key_init(void)
{
    
    

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

    key_io_init();
    atomic_set(&key.key_val,INVALIB_VALUE);
    atomic_set(&key.releasekey,0);
    return 0;
}


static int __exit key_exit(void)
{
    del_timer_sync(key.timer);

    unregister_chrdev_region(key.devid,1); /* 注销 */
    cdev_del(&key.cdev);
    class_destroy(key.class); 
    device_destroy(key.class,key.devid);

    free_irq(key.irqkey.irq_num,&key);
    

    return 0;
}


module_init(key_init);
module_exit(key_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lilone");



