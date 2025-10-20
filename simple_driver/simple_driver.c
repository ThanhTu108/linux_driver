#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/err.h>
//file opetation, cdev
#include <linux/fs.h>   
#include <linux/cdev.h>

//device file
#include <linux/kdev_t.h>
#include <linux/device.h>

//copy_from/to_user
#include <linux/uaccess.h>
//kmalloc
#include <linux/slab.h>

#define MEM_SIZE 1024

//global variable
dev_t dev_num = 0;
struct class* dev_class;
struct device* dev_file;
struct cdev cdev_fops;
uint8_t* kernel_buffer;


/*
** Function Prototypes
*/
static int __init simple_driver_init(void);
static void __exit simple_driver_exit(void);
static int open_fops(struct inode* inode, struct file* file);
static int release_fops(struct inode* inode, struct file* file);
static ssize_t read_fops(struct file* file, char __user* buf, size_t len, loff_t* off);
static ssize_t write_fops(struct file* file, const char __user* buf, size_t len, loff_t* off);

struct file_operations my_fops = 
{
    .owner = THIS_MODULE,
    .open = open_fops,
    .read = read_fops,
    .write = write_fops,
    .release = release_fops,
};

static int open_fops(struct inode* inode, struct file* file)
{
    pr_info("Open\n");
    return 0;
}
static int release_fops(struct inode* inode, struct file* file)
{
    pr_info("Close\n");
    return 0;
}

static ssize_t read_fops(struct file* file, char __user* buf, size_t len, loff_t* off)
{
    if(copy_to_user(buf, kernel_buffer, MEM_SIZE))
    {
        pr_err("Data read: err!!!\n");
    }
    pr_info("Read: Done!!!\n");
    return MEM_SIZE;
}
static ssize_t write_fops(struct file* file, const char __user * buf, size_t len, loff_t* off)
{
    if(copy_from_user(kernel_buffer, buf, len))
    {
        pr_err("Data write: err!!!\n");
    }
    pr_info("Write: Done!!!\n");
    return len;
}


static int __init simple_driver_init(void)
{
    if(alloc_chrdev_region(&dev_num, 0, 1, "simple_driver_mm")<0)
    {
        pr_err("Cannot allocate major and minor number!\n");
        return -1;
    }
    pr_info("\tMAJOR(%d)\n \tMINOR(%d)", MAJOR(dev_num), MINOR(dev_num));

    dev_class = class_create(THIS_MODULE, "simple_driver_class");
    if(IS_ERR(dev_class))
    {
        pr_err("Cannot create struct class for device file");
        goto r_class;
    }
    dev_file = device_create(dev_class, NULL, dev_num, NULL, "simple_driver_df");
    if(IS_ERR(dev_file))
    {
        pr_err("Cannot create device file");
        goto r_device;
    }
    cdev_init(&cdev_fops, &my_fops);
    if(cdev_add(&cdev_fops, dev_num, 1))
    {
        pr_err("Cannot add device to the system\n");
        goto r_class;
    }

    //Create physical memory 
    kernel_buffer = (uint8_t*)kmalloc(MEM_SIZE, GFP_KERNEL);
    if(!kernel_buffer)
    {
        pr_err("Cannot allocate memory in kernel");
        goto r_device;
    }
    strcpy(kernel_buffer, "hello kernel");
    pr_info("Init done(simple driver)\n");
    return 0;
r_device:
    class_destroy(dev_class);
r_class: 
    unregister_chrdev_region(dev_num, 1);
    return -1;
}

static void __exit simple_driver_exit(void)
{
    kfree(kernel_buffer);
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&cdev_fops);
    unregister_chrdev_region(dev_num, 1);
    pr_info("Remove done (simple driver)\n");
}

module_init(simple_driver_init);
module_exit(simple_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");

