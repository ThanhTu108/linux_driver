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
//copy
#include <linux/uaccess.h>
dev_t dev_num = 0;
static struct class* dev_class;
static struct cdev my_cdev;

char kernel_buf[20] = {0};
int val_export = 0; //export variable

void func_export(void);

EXPORT_SYMBOL_GPL(func_export);
EXPORT_SYMBOL_GPL(val_export);


static int __init cre_export_symbol(void);
static void __exit rmv_export_symbol(void);
static int open_fops(struct inode* inode, struct file* file);
static int release_fops(struct inode* inode, struct file* file);
static ssize_t read_fops(struct file* file, char __user* buf, size_t len, loff_t* off);
static ssize_t write_fops(struct file* file, const char __user* buf, size_t len, loff_t* off);

struct file_operations my_fops = 
{
    .owner = THIS_MODULE,
    .open = open_fops,
    .release = release_fops,
    .read = read_fops, 
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
    int ret;

    pr_info("READ\n");

    if (*off != 0)
        return 0;

    ret = snprintf(kernel_buf, sizeof(kernel_buf), "%d\n", val_export);

    if (copy_to_user(buf, kernel_buf, ret)) {
        pr_err("Cannot read\n");
        return -EFAULT;
    }

    *off += ret;
    return ret;
}

static ssize_t write_fops(struct file* file, const char __user* buf, size_t len, loff_t* off)
{
    pr_info("WRITE\n");
    if(copy_from_user(kernel_buf, buf,len))
    {
        pr_err("Cannot write from user\n");
        return -1;
    }
    sscanf(kernel_buf, "%d", &val_export);
    return len;
}

void func_export(void)
{
    pr_info("Export function from ex_exportsymbol.c \n");
}


static int __init cre_export_symbol(void)
{
    if(alloc_chrdev_region(&dev_num, 0, 1, "export_symbol_num") < 0)
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
    dev_class = class_create(THIS_MODULE, "export_symbol_class");
    if(IS_ERR(dev_class))
    {
        pr_err("Cannot create struct class \n");
        goto r_class;
    }
    if(IS_ERR(device_create(dev_class, NULL, dev_num, NULL, "export_symbol_device")))
    {
        pr_err("Cannot create device file\n");
        goto r_device;
    }
    pr_info("Insert example export symbol done \n");
    return 0;
r_device: 
    class_destroy(dev_class);
r_class:
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    return -1;
}
static void __exit rmv_export_symbol(void)
{   
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("Remove example export symbol done\n");
}

module_init(cre_export_symbol);
module_exit(rmv_export_symbol);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu10803@gmail.com");