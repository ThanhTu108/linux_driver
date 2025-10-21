#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
// device file
#include <linux/device.h>
#include <linux/kdev_t.h>
//fops
#include <linux/cdev.h>
#include <linux/fs.h>
//copy_from/to_user
#include <linux/uaccess.h>
//kmalloc, kfree
#include <linux/slab.h>

dev_t dev_num = 0;
struct class* dev_class;
struct device* dev_file;
struct cdev cdev_fops;

#define WR_VAL _IOW('a', 'a', int32_t* )
#define RD_VAL _IOR('a', 'b', int32_t* )
int32_t value = 0;

static int __init create_ioctl(void);
static void __exit remove_ioctl(void);
//int (*open) (struct inode *, struct file *);
static int open_func(struct inode* inode, struct file* file);
static int release_func(struct inode* inode, struct file* file);
static ssize_t read_func(struct file* file, char __user* buf, size_t len, loff_t* off);
static ssize_t write_func(struct file* file, const char __user* bud, size_t len, loff_t* off);
static long ioctl_func(struct file* file, unsigned int cmd, unsigned long arg);


static struct file_operations my_fops =
{
    .owner = THIS_MODULE,
    .open = open_func,
    .release = release_func,
    .read = read_func,
    .write = write_func,
    .unlocked_ioctl = ioctl_func,
};

static int open_func(struct inode* inode, struct file* file)
{
    pr_info("Open\n");
    return 0;
}
static int release_func(struct inode* inode, struct file* file)
{
    pr_info("Release\n");
    return 0;
}

static ssize_t read_func(struct file* file, char __user* buf, size_t len, loff_t* off)
{
    pr_info("Read func\n");
    return 0;
}
static ssize_t write_func(struct file* file,const char __user* buf, size_t len, loff_t* off)
{
    pr_info("Write func\n");
    return 0;
}
static long ioctl_func(struct file* file, unsigned int cmd, unsigned long arg)
{
    switch(cmd)
    {
        case WR_VAL:
            if(copy_from_user(&value, (int32_t*)arg, sizeof(value)))
            {
                pr_err("Data write: Err!!!\n");
            }
            pr_info("Value = %d", value);
            break;
        case RD_VAL:
            if(copy_to_user((int32_t*)arg, &value, sizeof(value)))
            {
                pr_err("Data read: Err!!!\n");
            }
            break;
        default: 
            pr_info(".......");
            break;
    }
    return 0;
}

static int __init create_ioctl(void)
{
    if(alloc_chrdev_region(&dev_num, 0, 1, "ioctl_mm") < 0)
    {
        pr_err("Cannot allocate major and minor number !\n");
        return -1;
    }
    pr_info("\tMajor(%d)\n\tMinor(%d)", MAJOR(dev_num), MINOR(dev_num));

    dev_class = class_create(THIS_MODULE, "ioctl_class");
    if(IS_ERR(dev_class))
    {
        pr_err("Cannot create struct class\n");
        goto r_class;
    }

    dev_file = device_create(dev_class, NULL, dev_num, NULL, "ioctl_device");
    if(IS_ERR(dev_file))
    {
        pr_err("Cannot create device file iotcl\n");
        goto r_device;
    }
    cdev_init(&cdev_fops, &my_fops);
    if(cdev_add(&cdev_fops, dev_num, 1) < 0)
    {
        pr_err("Cannot add device to the systems\n");
        goto r_class;
    }
    return 0;
r_device:
    class_destroy(dev_class);
r_class: 
    unregister_chrdev_region(dev_num, 1);
    return -1;
}
static void __exit remove_ioctl(void)
{
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&cdev_fops);
    unregister_chrdev_region(dev_num, 1);
    pr_info("Remove ioctl done!\n");
}

module_init(create_ioctl);
module_exit(remove_ioctl);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");
MODULE_DESCRIPTION("simple driver ioctl");