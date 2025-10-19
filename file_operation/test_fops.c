#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>

//Device file
#include <linux/kdev_t.h>
#include <linux/device.h>

//File operation
#include <linux/cdev.h>
#include <linux/fs.h>   

// Global variable
dev_t dev_num = 0;
static struct class* dev_class;
static struct device* dev_file;
static struct cdev cdev_fops;

/*
** Function Prototypes
*/

static int __init create_fops(void);
static void __exit remove_fops(void);
// int (*open) (struct inode *, struct file *);
// int (*release) (struct inode *, struct file *);
// ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
// ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
int open_func(struct inode*, struct file* file);
int release_func(struct inode* inode, struct file* file);
ssize_t read_func(struct file* , char __user* , size_t , loff_t* );
ssize_t write_func(struct file* , const char __user*, size_t, loff_t*);

static const struct file_operations my_fops =
{
    .owner = THIS_MODULE,
    .open = open_func,
    .release = release_func,
    .write = write_func,
    .read = read_func,
};

int open_func(struct inode*, struct file* file)
{
    pr_info("open function is call\n");
    return 0;
}

int release_func(struct inode* inode, struct file* file)
{
    pr_info("Release function\n");
    return 0;
}

ssize_t write_func(struct file* file, const char __user* buf, size_t len, loff_t* off)
{
    pr_info("write function is called\n");
    return len;
}

ssize_t read_func(struct file* , char __user* , size_t , loff_t* )
{
    pr_info("read function is called\n");
    return 0;
}

static int __init create_fops(void)
{
    if(alloc_chrdev_region(&dev_num, 0, 1, "mm_fops") < 0)
    {
        pr_err("Cannot allocate mm for fops");
        return -1;
    }
    pr_info("MAJOR(%d), MINOR(%d)", MAJOR(dev_num), MINOR(dev_num));
    dev_class = class_create(THIS_MODULE, "class_fops");
    if(IS_ERR(dev_class))
    {
        pr_err("Cannot create class for fops");
        goto r_class;
    }

    dev_file = device_create(dev_class, NULL, dev_num, NULL, "device_fops");
    if(IS_ERR(dev_file))
    {
        pr_err("Cannot create device file fops");
        goto r_device;
    }
    cdev_init(&cdev_fops, &my_fops);
    if(cdev_add(&cdev_fops, dev_num, 1) < 0)
    {
        pr_err("Cannot add device to system\n");
        goto r_class;
    }
    return 0;
r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev_num, 1);
    return -1;
}

static void __exit remove_fops(void)
{
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&cdev_fops);
    unregister_chrdev_region(dev_num, 1);
    pr_info("remove all\n");

}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("simple driver fops (cdev, fops)");

module_init(create_fops);
module_exit(remove_fops);