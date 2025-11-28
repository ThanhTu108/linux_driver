#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>   
#include <linux/err.h>
#include <linux/uaccess.h>  //copy_from/to_user
//device file
#include <linux/device.h>
#include <linux/kdev_t.h>
//fops
#include <linux/fs.h>
#include <linux/cdev.h>
//gpio
#include <linux/gpio.h>

//Define
#define GPIO_12 (12)

//global variable
dev_t dev_num = 0;
static struct class* dev_class;
static struct cdev my_cdev;

//global protocol
static int __init ex_gpio_initexit(void);
static void __exit rmv_gpio_initexit(void);

static int open_fops(struct inode* inode, struct file* file);
static int release_fops(struct inode* inode, struct file* file);
static ssize_t read_fops(struct file* file, char __user* user_buf, size_t len, loff_t* off);
static ssize_t write_fops(struct file* file, const char __user* user_buf, size_t len, loff_t* off);

static struct file_operations my_fops = 
{
    .owner = THIS_MODULE,
    .open = open_fops,
    .read = read_fops, 
    .write = write_fops,
    .release = release_fops,
};


static int open_fops(struct inode* inode, struct file* file)
{
    pr_info("OPEN\n");
    return 0;
}
static int release_fops(struct inode* inode, struct file* file)
{
    pr_info("RELEASE\n");
    return 0;
}
static ssize_t read_fops(struct file* file, char __user* user_buf, size_t len, loff_t* off)
{
    uint8_t gpio_state = 0;
    gpio_state = gpio_get_value(GPIO_12);
    len = 1; 
    if(copy_to_user(user_buf, &gpio_state, len))
    {
        pr_err("Read fail\n");
    }
    pr_info("Gpio 60 state = %d\n", gpio_state);
    return 0;
}
static ssize_t write_fops(struct file* file, const char __user* user_buf, size_t len, loff_t* off)
{
    uint8_t value[10] = {0};
    if(copy_from_user(&value[0], user_buf, len))
    {
        pr_err("Write fail\n");
    }
    pr_info("Set gpio_60 = %c\n", value[0]);
    if(value[0] == '1' || value[0] == 1)
    {
        gpio_set_value(GPIO_12, 1);
    }
    else if(value[0] == '0' || value[0] == 0)
    {
        gpio_set_value(GPIO_12, 0);
    }
    return len;
}


static int __init ex_gpio_initexit(void)
{
    if(alloc_chrdev_region(&dev_num, 0, 1, "gpio_basic_num") < 0)
    {
        pr_err("Cannot create major minor\n");
        return -1;
    }
    pr_info("Major(%d), Minor(%d)\n", MAJOR(dev_num), MINOR(dev_num));
    cdev_init(&my_cdev, &my_fops);
    if(cdev_add(&my_cdev, dev_num, 1) < 0)
    {
        pr_err("Cannot add device to system\n");
        goto r_class;
    }
    dev_class = class_create(THIS_MODULE, "gpio_basic_class");
    if(IS_ERR(dev_class))
    {
        pr_err("Cannot create struct class\n");
        goto r_class;
    }
    if(IS_ERR(device_create(dev_class, NULL, dev_num, NULL, "gpio_basic_device")))
    {
        pr_err("Cannot create device file\n");
        goto r_device;
    }
    if(gpio_is_valid(GPIO_12) == false)
    {
        pr_err("GPIO %d is not valid", GPIO_12);
        goto r_device;
    }
    pr_info("GPIO %d is valid", GPIO_12);
    
    if(gpio_request(GPIO_12, "debug_gpio60") < 0)
    {
        pr_err("Err request gpio %d\n", GPIO_12);
        goto r_gpio;
    }
    gpio_direction_output(GPIO_12, 0);
    gpio_export(GPIO_12, false);
    pr_info("Insert gpio basic done\n");
    return 0;
r_gpio:
    gpio_free(GPIO_12);
r_device:
    device_destroy(dev_class, dev_num);
r_class:
    class_destroy(dev_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    return -1;
}
static void __exit rmv_gpio_initexit(void)
{
    gpio_unexport(GPIO_12);
    gpio_free(GPIO_12);
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("remove basic gpio done\n");
}

module_init(ex_gpio_initexit);
module_exit(rmv_gpio_initexit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");