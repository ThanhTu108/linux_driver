#include <linux/fs.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/init.h>
//misc device
#include <linux/miscdevice.h>


//global prototype
static int __init cre_misc_device(void);
static void __exit rmv_misc_device(void);
static int open_fops(struct inode* inode, struct file* file);
static int release_fops(struct inode* inode, struct file* file);
static ssize_t read_fops(struct file* file, char __user* buf, size_t len, loff_t* off);
static ssize_t write_fops(struct file* file, const char __user* buf, size_t len, loff_t* off);

static struct file_operations my_fops =
{
    .owner = THIS_MODULE,
    .open = open_fops, 
    .release = release_fops,
    .read = read_fops, 
    .write = write_fops,
};

struct miscdevice my_miscdev = 
{
    .minor = MISC_DYNAMIC_MINOR, 
    .name = "misc_device_example",
    .fops = &my_fops,
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
    pr_info("READ\n");
    return 0;
}
static ssize_t write_fops(struct file* file, const char __user* buf, size_t len, loff_t* off)
{
    pr_info("WRITE\n");
    return len;
}


static int __init cre_misc_device(void)
{
    int ret = 0;
    ret = misc_register(&my_miscdev);
    if(ret)
    {
        pr_err("Cannot register misc device\n");
        return ret;
    }
    pr_info("Init misc device done\n");
    return 0;
}
static void __exit rmv_misc_device(void)
{
    misc_deregister(&my_miscdev);
    pr_info("Remove misc device done\n");
}

module_init(cre_misc_device);
module_exit(rmv_misc_device);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("thanhtu108303@gmail.com");