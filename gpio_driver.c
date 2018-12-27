#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/device.h>
#include<linux/cdev.h>
#include<linux/kdev_t.h>

#define LEDPORT 23
#define DEV_COUNT 1
#define DEVICE_NAME "gpio_driver"

static int state = 0;
static struct class *gpio_class;
static struct device *gpio_device;
struct cdev gpio_dev;

module_param(state,int,0660);

static int led_open(struct inode *pinode,struct file* pfile);
static int led_close(struct inode *pinode,struct file *pfile);



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
	int err;
	dev_t devno;
	unsigned int count = DEV_COUNT;

	printk(KERN_INFO "Initializing GPIO Driver\n");
	
	ret = alloc_chrdev_region(&devno,0,1,DEVICE_NAME);
	if(ret < 0)
	{
		printk(KERN_INFO "alloc_chrdev_region() failed\n");
		return -1;
	}
	
	printk(KERN_INFO "alloc_chrdev_region successfull\n");
	
	gpio_class = class_create(THIS_MODULE,DEVICE_NAME);
	if(gpio_class == NULL)
	{
		printk(KERN_INFO "Class create() failed\n");
		unregister_chrdev_region(devno,DEV_COUNT);
		return -1;
	}

	printk(KERN_INFO "class create successfull\n");

	gpio_device = device_create(gpio_class,NULL,devno,NULL,DEVICE_NAME);
	if(gpio_device == NULL)
	{
		printk(KERN_INFO "Device create() failed\n");
		class_destroy(gpio_class);
		unregister_chrdev_region(devno,DEV_COUNT);
		return -1;
	}
	
	printk(KERN_INFO "device create successfull\n");
	
	cdev_init(&gpio_dev,&led_ops);
	gpio_dev.owner = THIS_MODULE;
	err = cdev_add(&gpio_dev,devno,count);
	if(err < 0)
	{
		printk(KERN_INFO "cdev_add() failed\n");
		device_destroy(gpio_class,devno);
		class_destroy(gpio_class);
		unregister_chrdev_region(devno,DEV_COUNT);
		return -1;
	}
	
	printk(KERN_INFO "cdev_add() successfull\n");
	
	gpio_direction_output(LEDPORT,0);

	return 0;
}

static int __exit gpio_exit(void)
{
	printk(KERN_INFO "Unloading Module\n");
	cdev_del(&gpio_dev);
	device_destroy(gpio_class,devno);
	class_destroy(gpio_class);
	unregister_chrdev_region(devno,DEV_COUNT);
	printk(KERN_INFO "Unloaded Successfully.....Exiting GPIO Driver\n");
	return 0;
}

static int led_open(struct inode *pinode,struct file* pfile)
{
	return 0;
}

static int led_close(struct inode *pinode,struct file *pfile)
{
	return 0;
}


module_init(gpio_init);
module_exit(gpio_exit);
