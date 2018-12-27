#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/device.h>
#include<linux/cdev.h>
#include<linux/kdev_t.h>
#include<linux/gpio.h>
#include<linux/uaccess.h>

#define LEDPORT 23
#define DEV_COUNT 1
#define DEVICE_NAME "gpiodriver"
#define CLASS_NAME "gpioclass"
#define MAXLEN 5

struct gpio_device
{
	//struct kfifo fifo;
	char buffer[MAXLEN];
	struct cdev gpio_dev;
};


static int state = 0;
static struct class *gpio_class;
static struct gpio_device *priv_gpio;
static dev_t devno;
static int ret;


module_param(state,int,0660);

static int led_open(struct inode *pinode,struct file* pfile);
static int led_close(struct inode *pinode,struct file *pfile);
static ssize_t led_read(struct file *pfile,char __user *buffer,size_t len,loff_t *offset);
static ssize_t led_write(struct file *pfile,const char __user *buffer,size_t len,loff_t *offset);


static struct file_operations led_ops = 
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
	static struct device *gpio_device;
	//unsigned int count = DEV_COUNT;

	printk(KERN_INFO "Initializing GPIO Driver\n");
	
	ret = alloc_chrdev_region(&devno,0,DEV_COUNT,DEVICE_NAME);
	if(ret < 0)
	{
		printk(KERN_INFO "alloc_chrdev_region() failed\n");
		return -1;
	}
	
	printk(KERN_INFO "alloc_chrdev_region successfull\n");
	
	gpio_class = class_create(THIS_MODULE,CLASS_NAME);
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
	
	priv_gpio = (struct gpio_device*)kmalloc(sizeof(struct gpio_device), GFP_KERNEL);
	if(priv_gpio == NULL)
	{
		printk(KERN_INFO "[%s]: kmalloc() failed to alloc gpio_device priv gpio.\n", THIS_MODULE->name);
		return -1;
	}
	printk(KERN_INFO "kmalloc() succeed\n");
	memset(priv_gpio,0,sizeof(struct gpio_device));

	cdev_init(&priv_gpio->gpio_dev,&led_ops);
	//priv_gpio->gpio_dev.owner = THIS_MODULE;
	err = cdev_add(&priv_gpio->gpio_dev,devno,DEV_COUNT);
	if(err < 0)
	{
		printk(KERN_INFO "cdev_add() failed\n");
		device_destroy(gpio_class,devno);
		class_destroy(gpio_class);
		unregister_chrdev_region(devno,DEV_COUNT);
		return -1;
	}
	
	printk(KERN_INFO "cdev_add() successfull\n");
	
	gpio_direction_output(LEDPORT,1);

	return 0;
}

static void __exit gpio_exit(void)
{
	printk(KERN_INFO "Unloading Module\n");
	cdev_del(&priv_gpio->gpio_dev);
	kfree(priv_gpio);
	device_destroy(gpio_class,devno);
	class_destroy(gpio_class);
	unregister_chrdev_region(devno,DEV_COUNT);
	printk(KERN_INFO "Unloaded Successfully.....Exiting GPIO Driver\n");
}

static int led_open(struct inode *pinode,struct file* pfile)
{
	struct gpio_device *priv;
	priv = container_of(pinode->i_cdev, struct gpio_device, gpio_dev);
	pfile->private_data = priv;
	printk(KERN_INFO "[%s]: gpio_open() called for gpio\n", THIS_MODULE->name);
	return 0;
}

static int led_close(struct inode *pinode,struct file *pfile)
{
	printk(KERN_INFO "[%s]: gpio_close() called.\n", THIS_MODULE->name);
	return 0;
}


static ssize_t led_read(struct file *pfile,char __user *buffer,size_t len,loff_t *offset)
{
	char temp_buffer[5];
	int led_value;
	printk(KERN_INFO "Reading Led State\n");
	led_value = gpio_get_value(LEDPORT);
	sprintf(temp_buffer,"%1d",led_value);
	len = sizeof(temp_buffer);
	switch(temp_buffer[0])
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

static ssize_t led_write(struct file *pfile,const char __user *ubuffer,size_t len,loff_t *offset)
{
	//char temp_buffer[5];
	int max_bytes, bytes_to_write, nbytes;
	struct gpio_device *priv = (struct gpio_device*)pfile->private_data;
	printk(KERN_INFO "[%s]: gpio_write() called\n", THIS_MODULE->name);
	printk(KERN_INFO "Writing Led\n");
	
	max_bytes = MAXLEN - *offset;
	bytes_to_write = max_bytes < len ? max_bytes : len;
	if(bytes_to_write == 0)
		return -ENOSPC;
	nbytes = bytes_to_write - copy_from_user(priv->buffer + *offset, ubuffer, bytes_to_write);
	*offset = *offset + nbytes;
	printk(KERN_INFO "nbytes = %d | btw = %d | MAX_bytes = %d | buffer = %c\n",nbytes,bytes_to_write,max_bytes,priv->buffer[0]);
	switch(priv->buffer[0])
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
	
	return nbytes;
	
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("hdevi");
MODULE_DESCRIPTION("GPIO DRIVER for Beaglebone");

module_init(gpio_init);
module_exit(gpio_exit);
