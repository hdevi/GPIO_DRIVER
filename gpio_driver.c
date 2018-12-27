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
static dev_t devno;
static int ret;


module_param(state,int,0660);

static int led_open(struct inode *pinode,struct file* pfile);
static int led_close(struct inode *pinode,struct file *pfile);
static ssize_t led_read(struct file *pfile,char __user *buffer,size_t len,loff_t *offset);
static ssize_t led_write(struct file *pfile,const char __user *buffer,size_t len,loff_t *offset);


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


static ssize_t led_read(struct file *pfile,char __user *buffer,size_t len,loff_t *offset)
{
	char temp_buffer[5];
	int led_value;
	printk(KERN_INFO "Reading Led State\n");
	led_value = gpio_get_value(LEDPORT);
	sprintf(temp_buffer,"%d",led_value);
	len = sizeof(temp_buffer);
	switch(buffer[0])
	{
		case '0':
			printk(KERN_INFO "read 0\n");
			gpio_set_value(LEDPORT,0);
			break;
	
		case '1':
			printk(KERN_INFO "read 1\n");
			gpio_set_value(LEDPORT,1);
			break;
	}
	
	if(copy_to_user(buffer,temp_buffer,len))
		return -EFAULT;

	if(*offset == 0)
	{
		*offset += 1;
		return 1;
	}
	else
		return 0;

	printk(KERN_INFO "Read Execution completed\n");
	 	
}

static ssize_t led_write(struct file *pfile,const char __user *buffer,size_t len,loff_t *offset)
{
	char temp_buffer[5];
	printk(KERN_INFO "Writing Led\n");
	
	if(copy_from_user(temp_buffer,buffer,1))
		return -EFAULT;

	switch(buffer[0])
	{
		case '0':
			printk(KERN_INFO "Write 0\n");
			gpio_set_value(LEDPORT,0);
			break;
	
		case '1':
			printk(KERN_INFO "Write 1\n");
			gpio_set_value(LEDPORT,1);
			break;

		default:
			printk(KERN_INFO "Invalid Option\n");
			break;
	}
	
	printk(KERN_INFO "Write Execution completed\n");
	return len;
	
}

module_init(gpio_init);
module_exit(gpio_exit);
