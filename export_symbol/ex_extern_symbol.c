#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/err.h>
//device file
#include <linux/device.h>
#include <linux/kdev_t.h>
//fops
#include <linux/fs.h>
#include <linux/cdev.h>

//extern variable
extern int val_export;
extern void func_export(void);


dev_t dev_num;
static struct class* dev_class;
static struct cdev my_cdev;

static int __init cre_extern_symbol(void);
static void __exit rmv_extern_symbol(void);
static int open_fops(struct inode* inode, struct file* file);
static int release_fops(struct inode* inode, struct file* file);
static ssize_t read_fops(struct file* file, char __user* buf, size_t len, loff_t* off);
static ssize_t write_fops(struct file* file, const char __user* buf, size_t len, loff_t* off);

struct file_operations my_fops =
{
    .owner = THIS_MODULE,
    .read = read_fops,
    .open = open_fops,
    .release = release_fops,
    .write = write_fops,
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
static ssize_t read_fops(struct file* file, char __user* buf, size_t len, loff_t* off)
{
    pr_info("READ_2\n");
    func_export();
    pr_info("Value of val_export from device 1: %d", val_export);
    return 0;
}
static ssize_t write_fops(struct file* file, const char __user* buf, size_t len, loff_t* off)
{
    pr_info("WRITE\n");
    return len;
}



static int __init cre_extern_symbol(void)
{
    if(alloc_chrdev_region(&dev_num, 0, 1, "extern_symbol_num") < 0)
    {
        pr_err("Cannot create major minor\n");
        return -1;
    }
    pr_info("MAJOR(%d), MINOR(%d)\n", MAJOR(dev_num), MINOR(dev_num));
    cdev_init(&my_cdev, &my_fops);
    if(cdev_add(&my_cdev, dev_num, 1) < 0)
    {
        pr_err("Cannot add device to system\n");
        goto r_class;
    }
    dev_class = class_create(THIS_MODULE, "extern_symbol_class");
    if(IS_ERR(dev_class))
    {
        pr_err("Cannot create struct class \n");
        goto r_class;
    }
    if(IS_ERR(device_create(dev_class, NULL, dev_num, NULL, "extern_symbol_device")))
    {
        pr_err("Cannot create device file\n");
        goto r_device;
    }
    pr_info("Insert example extern symbol done \n");
    return 0;
r_device: 
    class_destroy(dev_class);
r_class:
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    return -1;
}


static void __exit rmv_extern_symbol(void)
{   
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("Remove example extern symbol done\n");
}

module_init(cre_extern_symbol);
module_exit(rmv_extern_symbol);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");