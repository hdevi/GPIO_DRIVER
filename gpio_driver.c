#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/device.h>

static struct fileoperations led_ops = 
{
	.owner = THIS_MODULE,
	.open = led_open,
	.read = led_read,
	.write = led_write,
	.release = led_close,
};

static int __init gpio_init(void)
{
	printk(KERN_INFO "Initializing GPIO Driver\n");
	return 0;
}

static int __exit gpio_exit(void)
{
	printk(KERN_INFO "Exiting GPIO Driver\n");
	return 0;
}

module_init(gpio_init);
module_exit(gpio_exit);
