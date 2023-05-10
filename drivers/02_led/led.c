#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <asm/mach/map.h>
#include <linux/printk.h>

#define LED_MAJOR 201
#define LED_NAME "led"

#define LED_OFF 0
#define LED_ON 1




#define CCM_CCGR1_BASE          (0X020C406C)  //CLK 
#define SW_MUX_GPIO1_IO03_BASE  (0X020E0068)
#define SW_PAD_GPIO1_IO03_BASE  (0X020E02F4)
#define GPIO1_DR_BASE           (0X0209C000)
#define GPIO1_GDIR_BASE         (0X0209C004)

static void __iomem *IMX6U_CCM_CCGR1;
static void __iomem *SW_MUX_GPIO1_IO03;
static void __iomem *SW_PAD_GPIO1_IO03;
static void __iomem *GPIO1_DR;
static void __iomem *GPIO1_GDIR;

void led_switch(int status)
{
    int reg_val = 0;
    reg_val = readl(GPIO1_DR);
    if(status == LED_ON)
    {
        reg_val &= ~(1<<3);  //GPIO1-IO3  low level open led
        writel(reg_val,GPIO1_DR);
    }
    else if(status == LED_OFF)
    {
        reg_val |= (1<<3);  //GPIO1-IO3  high level open led
        writel(reg_val,GPIO1_DR);
    }

}



ssize_t led_read(struct file *file, char __user *buf, size_t cnt, loff_t *off_t)
{
   
    return 0;
}

ssize_t led_write(struct file *file, const char __user *buf, size_t cnt, loff_t *off_t)
{
    unsigned char status[1];
    unsigned char led_sta;
    int ret;
    ret = copy_from_user(status,buf,cnt);
    if(ret < 0)
    {
        printk("kernel write failed!\r\n");
        return 0;
    }
    printk("status[0]=%s",status);
    led_sta = status[0];
    printk("led_sta=%s",led_sta);
    if(led_sta == LED_ON)
    {
        led_switch(LED_ON);
    }
    else if(led_sta == LED_OFF)
    {
        led_switch(LED_OFF);
    }



    return 0;
}


int led_open(struct inode *inode, struct file *file)
{
    printk("led open!\r\n");
    return 0;
}

int led_release(struct inode *inode, struct file *file)
{
    printk("led close!\r\n");
    return 0;
}


static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .read = led_read,
    .write = led_write,
    .release = led_release,
};



/* 驱动入口函数 */
static int __init led_init(void)
{
	/* 入口函数具体内容 */
	int retvalue = 0;
    int reg_val = 0;
	
    IMX6U_CCM_CCGR1 = ioremap(CCM_CCGR1_BASE,4);
    SW_MUX_GPIO1_IO03 = ioremap(SW_MUX_GPIO1_IO03_BASE, 4);
    SW_PAD_GPIO1_IO03 = ioremap(SW_PAD_GPIO1_IO03_BASE, 4);
    GPIO1_DR = ioremap(GPIO1_DR_BASE, 4);
    GPIO1_GDIR = ioremap(GPIO1_GDIR_BASE, 4);


    //enable CLK
    reg_val = readl(IMX6U_CCM_CCGR1);
    reg_val &= ~(3<<26);
    reg_val |= (3<<26); 
    writel(reg_val,IMX6U_CCM_CCGR1); 

    //复用
    writel(5,SW_MUX_GPIO1_IO03);  //0101  复用为gpio1

    //IO 
    writel(0x10B0, SW_PAD_GPIO1_IO03);
    /*
    reg_val = readl(SW_PAD_GPIO1_IO03);
    reg_val &= ~(1<<0);  //Slow Slew Rate

    reg_val &= ~(7<<3); //R0/6
    reg_val |= (6<<3);  

    reg_val &= ~(3<<6); //SPEED_2_medium_100MHz
    reg_val |= (2<<6);

    reg_val &= ~(1<<11);  //推挽

    reg_val &= ~(1<<12); //Pull/Keeper Enabled
    reg_val |= (1<<12);

    reg_val &= ~(1<<13);  //keeper

    reg_val &= ~(3<<14); //100k pull down
    writel(reg_val,SW_PAD_GPIO1_IO03);  */

    reg_val = readl(GPIO1_GDIR);
    reg_val &= ~(1 << 3); /* 清除以前的设置 */
    reg_val |= (1 << 3); /* 设置为输出 */
    writel(reg_val, GPIO1_GDIR);

    /* 5、默认关闭 LED */
    reg_val = readl(GPIO1_DR);
    reg_val |= (1 << 3); 
    writel(reg_val, GPIO1_DR);

    /* 注册字符设备驱动 */
	retvalue = register_chrdev(LED_MAJOR,LED_NAME, &led_fops);
	if(retvalue < 0){
	/* 字符设备注册失败,自行处理 */   
        printk("register failed!\r\n"); 
    }

    return 0;
}

/* 驱动出口函数 */
static void __exit led_exit(void)
{
	
    //取消映射
    iounmap(IMX6U_CCM_CCGR1);
    iounmap(SW_MUX_GPIO1_IO03);
    iounmap(SW_PAD_GPIO1_IO03);
    iounmap(GPIO1_DR);
    iounmap(GPIO1_GDIR);

    /* 注销字符设备驱动 */
    unregister_chrdev(LED_MAJOR, LED_NAME);

}




module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("lilone");









